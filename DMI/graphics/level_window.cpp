#include "level_window.h"
level_window::level_window() : input_window("Level")
{
    buttons[0] = new TextButton("Level 1", 102, 50, nullptr);
    buttons[1] = new TextButton("Level 2", 102, 50, nullptr);
    buttons[2] = new TextButton("Level 3", 102, 50, nullptr);
    buttons[3] = new TextButton("Level 0", 102, 50, nullptr);
    buttons[4] = new TextButton("LZB", 102, 50, nullptr);
    buttons[5] = new TextButton("EBICAB", 102, 50, nullptr);
    setLayout();
}