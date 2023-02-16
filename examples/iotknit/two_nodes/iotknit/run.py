from iotknit import *

led1Status = False

init("localhost")  # use the MQTT broker on localhost

prefix("led")  # all actors below are prefixed with /led

led1 = publisher("led1")


def button1Callback(msg):
    global led1Status

    print("received: [button]", msg)

    if (msg == "down"):
        led1Status = not led1Status  # toggle status of led

        if (led1Status):
            led1.publish("set", "on")  # publish updated state
            print("sending: [led1]", "on")
        else:
            led1.publish("set", "off")
            print("sending: [led1]", "of")


prefix("button")  # all sensors below are prefixed with /button

button1 = subscriber("button1")
button1.subscribe_change(callback=button1Callback)

run()
