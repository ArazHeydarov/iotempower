rgb_strip
=========

..  code-block:: cpp

    rgb_strip(name, num_leds, Striptype, data_pin, 
        ...other FastLed parameters...);

Create a new rgb strip device object for an rgb-led strip connected to the
given ``data_pin`` (or ``data`` and ``clock_pin``) with ``num_leds`` rgb leds.
All led chips supported by FastLED (https://github.com/FastLED/FastLED) are
supported.

Topics
------

Name will be appended to the mqtt topic.

Under the resulting topic will be the following subtopics:

- ``set``: turn on or off

- ``brightness``, ``brightness/set``: read and set brightness (0-255)

- ``rgb``, ``rgb/set``:
  read and set color as colorname, 6-digit hexcode
  ``rrggbb`` or comma separated triplet ``r,g,b``

  This also takes a number to just affect one led
  It also takes the words front and back, which will
  push the new color into strip (from the front or the
  back) and move all other led-colors by one.

Parameters
----------

- ``name``: the name it can be addressed via MQTT in the network.
  Inside the code
  it can be addressed via IN(name).

- ``data_pin``: the data-pin the led strip is connected to

- ``num_led``: number of leds on strip

- ``...other FastLed parameters...``: data related to chip configuration (use
  same format as in pointy brackets in ``FastLED.addLeds<>``).

Example
-------

**node name:** ``living room/tvlights``

..  code-block:: cpp

    rgb_strip(strip2, 10, WS2811, D6, BRG);

Now the second RGB LED can be switched to red via sending to the mqtt-broker
to the topic ``living room/tvlights/strip2/rgb/set`` the command ``2 red`` or
``front red``.

Check also `rgb_matrix <rgb_matrix.rst>`_ and `animator <animator.rst>`_.
