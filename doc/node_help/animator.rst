TODO
So far just an example:

..  code-block:: cpp

    /* setup.cpp */

    //////// Global Variables ////////
    unsigned long frames[4]={0,0,0,0};
    enum anim_type {none, fade, fade_to, scroll};
    anim_type anim_types[4]={none, none, none, none};


    //////// Device setup  ////////
    // RGB strips
    rgb_strip(strip1, 50, WS2811, D3, BRG);
    rgb_strip(strip2, 50, WS2811, D5, BRG);
    rgb_strip(strip3, 50, WS2811, D4, BRG);
    rgb_strip(strip4, 50, WS2811, D1, BRG);

    // A matrix consisting of several strips
    rgb_matrix(matrix, "matrix", 25, 4)
            .with(IN(strip1), 0, 0, Right_Down, 25)
            .with(IN(strip2), 0, 1, Right_Down, 25)
            .with(IN(strip3), 0, 2, Right_Down, 25)
            .with(IN(strip4), 0, 3, Right_Down, 25);

    // An animator object with support functions (this one animates the matrix)
    void draw_pattern(int p, int line, int len) {
        switch(p) {
            case 1:
                IN(matrix).rainbow(0, line, len, 1);
                break;
            case 2:
                IN(matrix).gradient_row(CRGB::Green, CRGB::Blue, 0, line, len, 1);
                break;
            case 3:
                IN(matrix).gradient_row(CRGB::Blue, CRGB::Red, 0, line, len, 1);
                break;
            default:
                break;
        }
    }

    void set_animation(Ustring& command, anim_type at, int leds, int frame_count) {
        int stripnr = limit(command.as_int() - 1, 0, 3);
        command.strip_param();
        int pattern = limit(command.as_int(), 1, 3);
        draw_pattern(matrix, pattern, stripnr, -1);
        anim_types[stripnr] = at;
        frames[stripnr] = frame_count;
    }

    animator(anim, IN(matrix))
        .with_fps(10)
        .with_frame_builder( [] () {
            for(int i=0; i<4; i++) {
                if(frames[i]>0 && anim_types[i] != none) {
                    switch(anim_types[i]) {
                        case fade:
                            IN(matrix).fade(8, 0, i, -1, 1);
                            break;
                        case scroll:
                            IN(matrix).scroll_right(false,i);
                            break;
                        case fade_to:
                            IN(matrix).fade_to(CRGB::Red, 16, 0, i, -1, 1);
                        default:
                            break; 
                    }
                    frames[i] --;
                }
            }
        } ).with_command_handler( "fade", [] (Ustring& command) {
            set_animation(command, fade, -1, 100);
        } ).with_command_handler( "fade_to", [] (Ustring& command) {
            set_animation(command, fade_to, -1, 50);
        } ).with_command_handler( "scroll", [] (Ustring& command) {
            set_animation(command, scroll, 5, 150);
        } );
