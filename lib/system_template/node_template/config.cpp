/* config.cpp
This is the configuration code for a ulnoiot node.
Here, you define all the devices (and eventually their interactions)
connected to the node specified with this directory.
*/

// Always start include this to make everything from ulnoiot available.
// Therefore, do not delete the following line.
#include <ulnoiot.h>

// The following is just example code, adjust to your needs accordingly.

// wifi and network connect are done automatically
// We assume for the example below that node topic is myroom/test1

// You might need to define some callback funtions here (for filters
// or interactions).

// The following line needs to always exist for syntactical reasons
void setup() {
    // All device setup/configuration needs to be done here,
    // look for the following commented examples for reference
    // and uncomment and modify as needed
    // check the output of uhelp at the ulnoiot command prompt for other
    // supported devices and options.

    // The onboard-led is always available.
    // With this configuration it will report under myroom/test1/blue
    // and can be set via sending off or on to myroom/test1/blue/test.
    // Device blue = output("blue", onboardled, "off", "on");
    // blue.high();
    
    // Add some other devices (all commented out)
    // Add a button with a slightly higher debounce rate, which will report
    // in the topic myroom/test1/button1.
    // button("button1", d8).withThreshold(2);
    
    // Count rising signals on d2=Pin(4) and
    // report number counted at myroom/test1/shock1.
    // trigger("shock1", 4);

} // end setup <- This line needs to be also kept