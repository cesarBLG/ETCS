#include "level_window.h"
#include "../monitor.h"
#include "../tcp/server.h"
#include "keyboard.h"
level_window::level_window() : input_window("Level", 1)
{
    inputs[0] = new level_input();
    create();
}
void level_window::sendInformation()
{
    string data = inputs[0]->getData();
    data = data.substr(6,1);
    write_command("setLevel",data);
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
    keys = getSingleChoiceKeyboard({"Level 1", "Level 2", "Level 3", "Level 0", "LZB", "EBICAB"}, this);
}
void level_input::validate()
{
    if(data.size() < 7) return;
    setAccepted(true);
}