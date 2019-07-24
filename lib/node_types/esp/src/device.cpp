// device.cpp
// author: ulno
// created: 2018-07-16

#include <Arduino.h>
#include "device.h"


void Subdevice::init(const char* subname, bool subscribed) {
    ulog("subdevice init: subname: >%s< subscribed: %d", subname, subscribed);
    name.from(subname);
    _subscribed = subscribed;
}

bool Subdevice::call_receive_cb(Ustring& payload) {
    Serial.print("Calling receive callback ");
    Serial.print(name.as_cstr());
    Serial.print(" ");
    if(receive_cb != NULL) {
        Serial.println(payload.as_cstr());
        return receive_cb(payload);
    }
    Serial.println("!failure!");
    return false;
}

//#include <device-manager.h>
// Just define add_device from here - to avoid messed up dependencies
bool add_device(Device& device);

Device::Device(const char* _name) {
    name.from(_name);
    ulog("Device: Adding device: %s", name.as_cstr());
    if(!add_device(*this)) {
        ulog("Device %s couldn't be added - max devices reached.", _name);
    }
}

#ifdef mqtt_discovery_prefix
    void Device::create_discovery_info(const String type,
            bool state_topic, const char* state_on, const char* state_off,
            bool command_topic, const String extra_json) { // TODO: for inverted on/off this doesn't really work -> rethink
        String slash("/");
        String name_s(name.as_cstr());
        String topic_s = String(mqtt_topic) + slash + name_s;
        String hostname_s(HOSTNAME);

        discovery_config_topic = mqtt_discovery_prefix + slash + type + slash
            + hostname_s + slash + name_s + String("/config");

        discovery_info = String("{ \"name\": \"") + String(name.as_cstr()) + String("\"");
        if(state_topic) {
            discovery_info += String(", \"state_topic\": \"") + topic_s 
                + String("\", \"state_on\": \"") + String(state_on) 
                + String("\", \"state_off\": \"") + String(state_off) + String("\"");
            if(command_topic) {
                discovery_info += String(", \"command_topic\": \"") + topic_s 
                    + String("/set\", \"payload_on\": \"") + String(state_on) 
                    + String("\", \"payload_off\": \"") + String(state_off) + String("\"");
            }
        }
        if(extra_json.length() > 0) {
            discovery_info += String(", ") + extra_json;
        }
        discovery_info += String(" }");
        
        // Serial.print("discovery topic: ");
        // Serial.println(discovery_config_topic);
        // Serial.print("discovery info: ");
        // Serial.println(discovery_info);
    }
#endif


bool Device::publish(AsyncMqttClient& mqtt_client, Ustring& node_topic) {
    bool published = false;
    Ustring topic;
    bool first = true;
    subdevices_for_each( [&] (Subdevice& sd) {
        const Ustring& val = sd.value();
        if(!val.empty()) {
            // construct full topic
            topic.copy(node_topic);
            topic.add("/");
            topic.add(get_name());
            const Ustring& sd_name = sd.get_name();
            if(!sd_name.empty()) { // only add, if name given
                topic.add("/");
                topic.add(sd.get_name());
            }
            if(first) {
                Serial.print(" ");
                first = false;
            }
            else Serial.print("|");
            Serial.print(topic.as_cstr()+node_topic.length()+1);
            Serial.print(":");
            Serial.print(val.as_cstr());

            if(!mqtt_client.publish(topic.as_cstr(), 0, true, val.as_cstr())) {
                Serial.print("!publish error!");
                // TODO: signal error and trigger reconnect - necessary?
                return false;
            }
            published = true;
        }
        return true; // continue loop
    } ); // end for_each - iterate over subdevices
    if(published) _needs_publishing = false;
    return published;
}

#ifdef mqtt_discovery_prefix
bool Device::publish_discovery_info(AsyncMqttClient& mqtt_client) {
    if(!mqtt_client.publish(discovery_config_topic.c_str(), 0, true, discovery_info.c_str())) {
        Serial.print("!discovery publish error!");
        // TODO: signal error and trigger reconnect - necessary?
        return false;
    }
    return true;
}
#endif



bool Device::poll_measure() {
    if(started()) { // only if device active
        if( micros() - last_poll >= _pollrate_us) { // only measure so often
            // TODO: reenable measure_init(); // something might have to be executed here to init each time, for example i2c setup
            if(!measure()) {  // measure new value or trigger physical update
                // re-use last value(s), if measurement not successful
                subdevices.for_each( [](Subdevice& sd) {
                    sd.measured_value.copy(sd.current_value);
                    return true; // continue loop
                } );
            } else {
                if(_ignore_case) { // if necessary, lower them all
                    subdevices.for_each( [](Subdevice& sd) {
                        sd.measured_value.lower();
                        return true; // continue loop
                    } );
                }
            }
            last_poll = micros();
            
            // a current value is now in measured_value(s)
            // check if it needs to be filtered
            if(_filter_cb != NULL && ! _filter_cb()) {
                // if filter defined but indicates to ignore the current measurement
                return false; // end here with no update
            }
            // The measured value(s) is/are now an actual, valid new measurement
            return true;
        }
    }
    return false;
}

bool Device::check_changes() {
    // check if the value has changed/is updated and call on_change_cb
    bool updated = false;
    bool changed = false;
    
    subdevices.for_each( [&] (Subdevice& sd) {
        if(!sd.measured_value.equals(sd.current_value)) {
            sd.current_value.copy(sd.measured_value);
            changed = true;
            if(_report_change) {
                updated = true;
                _needs_publishing = true;
                Serial.print("Needs publishing: ");
                Serial.print(name.as_cstr());
                if(!sd.get_name().empty()) {
                    Serial.print("/");
                    Serial.print(sd.get_name().as_cstr());
                }
                Serial.print(":");
                Serial.println(sd.measured_value.as_cstr());
            }
        }
        return true; // continue loop
    } ); // end for each subdevices

    if(changed) {
        call_on_change_callback();
    }
    return updated;
}

static Ustring value_error;

// TODO: why can't I make this const/const?
Ustring& Device::value(unsigned long index) {
    Subdevice* sd = subdevice(index);
    if(sd == NULL) {
        ulog("Error in value(). Device: %s", name.as_cstr());
        value_error.from("subdevice value error");
        return value_error;
        // TODO: should this crash here?
    }
    return sd->value();
}

Ustring& Device::measured_value(unsigned long index) {
    Subdevice* sd = subdevice(index);
    if(sd == NULL) {
        ulog("Error in measured_value(): Device: %s", name.as_cstr());
        value_error.from("no subdevice");
        return value_error;
    }
    return sd->measured_value;
}
