#include "level_window.h"
#include "../monitor.h"
level_window::level_window() : input_window("Level", 1)
{
    inputs[0] = new level_input();
    setLayout();
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
    keys.push_back(new TextButton("Level 1", 102, 50));
    keys.push_back(new TextButton("Level 2", 102, 50));
    keys.push_back(new TextButton("Level 3", 102, 50));
    keys.push_back(new TextButton("Level 0", 102, 50));
    keys.push_back(new TextButton("LZB", 102, 50));
    keys.push_back(new TextButton("EBICAB", 102, 50));
    for(int i=0; i<4; i++)
    {
        keys[i]->setPressedAction([this, i]
        {
            if(i<3) data = "Level " + to_string(i+1);
            if(i==3) data = "Level 0";
        });
    }
}
void level_input::validate()
{
    //TODO: manual level selection requires confirmation
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
}