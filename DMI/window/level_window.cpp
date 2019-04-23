#include "level_window.h"
#include "../monitor.h"
#include "keyboard.h"
level_window::level_window() : input_window("Level", 1)
{
    inputs[0] = new level_input();
    create();
}
level_input::level_input()
{
    switch(level)
    {
        case N0:
            data = "Level 0";
            break;
        case N1:
            data = "Level 1";
            break;
        case N2:
            data = "Level 2";
            break;
        case N3:
            data = "Level 3";
            break;
    }
    accepted = true;
    keys = getSingleChoiceKeyboard({"Level 1", "Level 2", "Level 3", "Level 0", "LZB", "EBICAB"}, data_set);
}
void level_input::validate()
{
    //TODO: manual level selection requires confirmation
    if(data.size() < 7) return;
    switch(data[6])
    {
        case '0':
            level = N0;
            break;
        case '1':
            level = N1;
            break;
        case '2':
            level = N2;
            break;
        case '3':
            level = N3;
            break;
    }
    setAccepted(true);
}