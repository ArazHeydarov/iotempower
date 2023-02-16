# install from https://github.com/PyUserInput/PyUserInput
from pykeyboard import PyKeyboard

from iotknit import *

kb = PyKeyboard()


def inject_key(key, msg):
    global kb
    if msg == "1":
        kb.press_key(key)
    else:
        kb.release_key(key)


def back_cb(msg):
    inject_key(kb.left_key, msg)

def forward_cb(msg):
    inject_key(kb.space, msg)


init("192.168.12.1")

prefix("remote1")
subscriber("forward").subscribe_change(callback=forward_cb)
subscriber("back").subscribe_change(callback=back_cb)

onboard_blue = switch_publisher("blue", init_state="off")

print("Keyboard injector started, press Ctrl-C to stop.")
run()
