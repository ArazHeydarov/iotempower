// device.h
// author: ulno
// created: 2018-07-16
//
// One iotempower device (actor or sensor connected to node)

// General note, in iotempower, we pass as convenience all values as strings.
// So other types have to be converted in such a as string or from one.

#ifndef _IOTEMPOWER_DEVICE_H_
#define _IOTEMPOWER_DEVICE_H_

#include <functional>
////AsyncMqttClient disabled in favor of PubSubClient
//#include <AsyncMqttClient.h>
#include <PubSubClient.h>
#include <iotempower-default.h>
#include <toolbox.h>

#include "config-wrapper.h"


class Subdevice {
    private:
        Ustring name; // subdevice name (added to the device name)
        bool _subscribed = false;
        void init_log();
        void init(const char* subname, bool subscribed);
        void init(const __FlashStringHelper* subname, bool subscribed);
        #define IOTEMPOWER_ON_RECEIVE_CALLBACK std::function<bool(const Ustring&)>
        IOTEMPOWER_ON_RECEIVE_CALLBACK receive_cb = NULL;
    public:
        Ustring measured_value; // the just measured value (after calling measure)
        Ustring current_value;
        Ustring& value() { return current_value; }
        Ustring& get() { return current_value; }
        Subdevice& with_receive_cb(IOTEMPOWER_ON_RECEIVE_CALLBACK cb) {
            receive_cb = cb;
            return *this;
        }
        bool call_receive_cb(Ustring& payload);
        const Ustring& get_name() const { return name; }
        const Ustring& key() const { return name; }
        bool subscribed() { return _subscribed; }
        Subdevice(const char* subname, bool subscribed) {
            init(subname, subscribed);
        }
        Subdevice(const char* subname) { 
            init(subname, false);
        }
        Subdevice(const __FlashStringHelper* subname, bool subscribed) {
            init(subname, subscribed);
        }
        Subdevice(const __FlashStringHelper* subname) { 
            init(subname, false);
        }
};

class Device {
    protected:
        Fixed_Map<Subdevice, IOTEMPOWER_MAX_SUBDEVICES> subdevices;
#ifdef mqtt_discovery_prefix
        // TODO: might have to move this to subdevice (if there are two values measured)
        String discovery_config_topic;
        String discovery_info;
        void create_discovery_info(const String& type,
            bool state_topic=true, const char* state_on=NULL, const char* state_off=NULL,
            bool command_topic=false, const char* payload_on=NULL, const char* payload_off=NULL,
            const String& extra_json=String());
#endif
    private:
        Ustring name; // device name and mqtt-topic extension
        bool _ignore_case = true;
        bool _report_change = true;
        bool _needs_publishing = false;

        // This is the callback which is called based on a value change
        // it gets passed the triggering device. Last measured values can be
        // read from the globally defined device.
        // TODO: commented example
        #define IOTEMPOWER_ON_CHANGE_CALLBACK std::function<void()>
        IOTEMPOWER_ON_CHANGE_CALLBACK _on_change_cb=NULL;

        // This is the callback which is used for filtering and influencing values
        // It does not get the current device passed as parameter. The value can
        // be read from measured_value in the globally defined device. This
        // measure_value can be now modified. It is declared valid via
        // returning true and invalid via returning false or returning an empty 
        // string (set first char to 0).
        // TODO: commented example
        #define IOTEMPOWER_FILTER_CALLBACK std::function<bool()>
        IOTEMPOWER_FILTER_CALLBACK _filter_cb=NULL;

        unsigned long _pollrate_us = 0; // poll as often as possible
        unsigned long last_poll = micros(); // when was last polled

    protected:
        bool _started = false;

    public:
        Device(const char* _name, unsigned long pollrate_ms);
        //// Getters & Setters
        Device& with_ignore_case(bool ignore_case) { 
            _ignore_case = ignore_case;
            return *this;
        }
        Device& set_ignore_case(bool ignore_case) {
            return with_ignore_case(ignore_case);
        }

        bool get_ignore_case() {
            return _ignore_case;
        }

        void call_on_change_callback() {
            if(_on_change_cb != NULL) {
                _on_change_cb();
            }
        }

        bool needs_publishing() {
            return _needs_publishing;
        }

        // publish current value(s) and resets needs_publishing state
        ////AsyncMqttClient disabled in favor of PubSubClient
        //bool publish(AsyncMqttClient& mqtt_client, Ustring& node_topic);
        bool publish(PubSubClient& mqtt_client, Ustring& node_topic);

#ifdef mqtt_discovery_prefix
        ////AsyncMqttClient disabled in favor of PubSubClient
        // bool publish_discovery_info(AsyncMqttClient& mqtt_client);
        bool publish_discovery_info(PubSubClient& mqtt_client);
#endif

        Ustring& value(unsigned long index);
        Ustring& value() { return value(0); }
        Ustring& measured_value(unsigned long index);
        Ustring& measured_value() { return measured_value(0); }
        const Ustring& get_name() const { return name; }
        //const char* get_name() const { return name.as_cstr(); }
        const Ustring& key() const { return name; }
        
        virtual ~Device() {
            // usually nothing has to be done here
            // This virtual method needs to be defined to prevent warning
            // from cpp-compiler
        }

        // pollrate
        Device& set_pollrate(unsigned long rate_ms) { 
            _pollrate_us = rate_ms * 1000;
            return *this;
        }
        Device& with_pollrate(unsigned long rate_ms) { 
            return set_pollrate(rate_ms);
        }
        Device& pollrate(unsigned long rate_ms) { 
            return set_pollrate(rate_ms);
        }
        Device& set_pollrate_us(unsigned long rate_us) { 
            _pollrate_us = rate_us;
            return *this;
        }
        Device& with_pollrate_us(unsigned long rate_us) { 
            return set_pollrate(rate_us);
        }
        Device& pollrate_us(unsigned long rate_us) { 
            return set_pollrate(rate_us);
        }

        // report_change
        Device& set_report_change(bool report_change) { 
            _report_change = report_change;
            return *this;
        }
        Device& with_report_change(bool report_change) {
            return set_report_change(report_change);
        } 
        Device& report_change(bool report_change=true) {
            return set_report_change(report_change);
        } 

        // on_change_callback
        Device& set_on_change_callback(IOTEMPOWER_ON_CHANGE_CALLBACK on_change_cb) { 
            _on_change_cb = on_change_cb;
            return *this;
        }
        Device& with_on_change_callback(IOTEMPOWER_ON_CHANGE_CALLBACK on_change_cb) { 
            return set_on_change_callback(on_change_cb);
        }
        Device& on_change_callback(IOTEMPOWER_ON_CHANGE_CALLBACK on_change_cb) { 
            return set_on_change_callback(on_change_cb);
        }
        Device& set_on_change(IOTEMPOWER_ON_CHANGE_CALLBACK on_change_cb) { 
            return set_on_change_callback(on_change_cb);
        }
        Device& with_on_change(IOTEMPOWER_ON_CHANGE_CALLBACK on_change_cb) { 
            return set_on_change_callback(on_change_cb);
        }
        Device& on_change(IOTEMPOWER_ON_CHANGE_CALLBACK on_change_cb) { 
            return set_on_change_callback(on_change_cb);
        }

        // filter_callback
        Device& set_filter_callback(IOTEMPOWER_FILTER_CALLBACK filter_cb) { 
            _filter_cb = filter_cb;
            return *this;
        }
        Device& with_filter_callback(IOTEMPOWER_FILTER_CALLBACK filter_cb) { 
            return set_filter_callback(filter_cb);
        }
        Device& filter_callback(IOTEMPOWER_FILTER_CALLBACK filter_cb) { 
            return set_filter_callback(filter_cb);
        }
        Device& with_filter(IOTEMPOWER_FILTER_CALLBACK filter_cb) { 
            return set_filter_callback(filter_cb);
        }
        Device& set_filter(IOTEMPOWER_FILTER_CALLBACK filter_cb) { 
            return set_filter_callback(filter_cb);
        }
        Device& filter(IOTEMPOWER_FILTER_CALLBACK filter_cb) { 
            return set_filter_callback(filter_cb);
        }

        Subdevice* add_subdevice(Subdevice* sd) {
            ulog(F("add_subdevice: device: %s subdev: >%s<"), name.as_cstr(), sd->get_name().as_cstr());
            if(subdevices.add(sd)) {
                ulog(F("add_subdevice >%s< succeeded."), sd->get_name().as_cstr());
                return sd;
            } else {
                ulog(F("add_subdevice >%s< failed."), sd->get_name().as_cstr());
                return NULL;
            }
        }

        Subdevice* subdevice(unsigned long index) {
            return subdevices.get(index);
        }

        bool subdevices_for_each(std::function<bool(Subdevice&)> func) {
            return subdevices.for_each(func);
        }

        bool started() { return _started; }

        /* poll_measure
         * This calls measure and filters and sets if necessary default values. 
         * This will be called very often from event loop
         * Will return if current measurement was successful or invalid. 
         * */
        bool poll_measure();

        /* check_changes
         * returns True if new values were measured and sets the publishing flag
         * This will be called very often from event loop 
         * This only reports change when the measured (and filtered) 
         * value is new (has changed) and report_changes is set.
         * If a change happened and a callback is set, the callback is called.
         * */
        bool check_changes();

        /* measure_init
         * Can be overloaded to allow re-initialization of some hardware
         * elements. For example a respective i2c bus due to a very weird
         * implementation of TwoWire in Wire.h
         * */
        virtual void measure_init() {}; // usually do nothing here

        /* start
         * This usually needs to be overwritten
         * Any physical initialization has to go here.
         * This is executed by the system right after all the rest has been 
         * intialized - right before the first subscription.
         * If something with the physical initialization goes wrong, started
         * should stay false. If physical intitialization worked this method
         * needs to set _started to true;
         * TODO: make protected?
         * */
        virtual void start() { _started = true; }

        /* measure
         * Does nothing by default.
         * Usually this needs to be overwritten.
         * It should make sure to trigger necessary steps to
         * measure from the physical hardware a sensor value (or
         * several values from a multi sensor).
         * It needs to set in measured_value the currently measured value.
         * If no value can be measured (or has not been), it should return
         * false. If measuring was successful, return True.
         * This is called from update to trigger the actual value generation.
         * If device is not marked as started, measure will not be called.
         * TODO: make protected?
         * */
        virtual bool measure() { return true; }
};


//// Some filters
// TODO: check how to turn this in a functor class that does not die when used
#define filter_average(TYPE, buflen, dev) [&] { \
        static TYPE sum = 0; \
        static size_t values_count = 0; \
        TYPE v; \
        if(std::is_same<TYPE, double>::value) { \
            v = dev.measured_value().as_float(); \
        } else { \
            v = dev.measured_value().as_int(); \
        } \
        sum += v; \
        values_count ++; \
        if(values_count >= buflen) { \
            dev.measured_value().from(sum/values_count); \
            values_count = 0; \
            sum = 0; \
            return true; \
        } \
        return false; \
    }



/* The Jeff McClintock running median estimate. 
 * base: https://stackoverflow.com/questions/10930732/c-efficiently-calculating-a-running-median
 * */
#define filter_jmc_median(update_ms, dev) [&] { \
        static double median = 0.0; \
        static double average = 0.0; \
        static unsigned long last_time = millis(); \
        double sample = dev.measured_value().as_float(); \
        average += ( sample - average ) * 0.1; \
        median += copysign( average * 0.01, sample - median ); \
        unsigned long current = millis() ; \
        if(current - last_time >= update_ms) { \
            dev.measured_value().from(average); \
            last_time = current; \
            return true; \
        } \
        return false; \
    }

/* Derivation of jmc median over small time intervals */
#define filter_jmc_interval_median(interval, dev) [&] { \
        static double median; \
        static double average; \
        static bool undefined = true; \
        static unsigned long start_time; \
        if(undefined) { \
            start_time = millis(); \
            median = 0.0; \
            average = 0.0; \
            undefined = false; \
        } \
        double sample = dev.measured_value().as_float(); \
        average += ( sample - average ) * 0.1; \
        median += copysign( average * 0.01, sample - median ); \
        unsigned long current = millis() ; \
        if(current - start_time >= interval) { \
            dev.measured_value().from(average); \
            undefined = true; /* trigger reset */ \
            return true; \
        } \
        return false; \
    }

/* Turn analog values into binary with a threshold level */
#define filter_binarize(cutoff, high, low, dev) [&] { \
        if(dev.measured_value().equals(low)) return false; \
        if(dev.measured_value().equals(high)) return false; \
        double sample = dev.measured_value().as_float(); \
        if(sample>=cutoff) { \
            dev.measured_value().from(high); \
        } else { \
            dev.measured_value().from(low); \
        } \
        return true; \
    }

/* round to the next multiple of base */
#define filter_round(base, dev) [&] { \
        long v = (long)(dev.measured_value().as_float()/(base)+0.5); \
        dev.measured_value().from(v*(base)); \
        return true; \
    }

/* return maximum one value per time interval (interval in ms) */
#define filter_limit_time(interval, dev) [&] { \
        static unsigned long last_time; \
        unsigned long current = millis() ; \
        if(!dev.measured_value().empty() \
           && current - last_time >= interval) { \
            last_time = current; \
            return true; \
        } \
        return false; \
    }


#endif // _IOTEMPOWER_DEVICE_H_
