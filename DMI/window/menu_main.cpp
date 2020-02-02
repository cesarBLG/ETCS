#include "menu_main.h"
#include "window.h"
#include "level_window.h"
#include "driver_id.h"
#include "running_number.h"
#include "train_data.h"
#include <algorithm>
#include "../monitor.h"
#include "../messages/messages.h"
#include "../time.h"
#include "../control/control.h"
#include "../tcp/server.h"

menu_main::menu_main() : menu("Main")
{
    buttons[0] = new TextButton("Start", 153, 50);
    buttons[1] = new TextButton("Driver ID", 153, 50);
    buttons[2] = new TextButton("Train Data", 153, 50);
    buttons[3] = new TextButton("Maintain shunting", 153, 50);
    buttons[4] = new TextButton("Level", 153, 50);
    buttons[5] = new TextButton("Train running number", 153, 50);
    buttons[6] = new TextButton("Shunting", 153, 50);
    buttons[7] = new TextButton("Non-Leading", 153, 50);
    buttons[3]->enabled = false;
    buttons[0]->setPressedAction([this]
    {
        write_command("startMission","");
        exit(this);
    });
    buttons[1]->setPressedAction([this] 
    {
        right_menu(new driver_window());
    });
    buttons[2]->setPressedAction([this] 
    {
        right_menu(new train_data_window());
    });
    buttons[4]->setPressedAction([this] 
    {
        right_menu(new level_window());
    });
    buttons[5]->setPressedAction([this] 
    {
        right_menu(new trn_window());
    });
    buttons[6]->setPressedAction([this]
    {
        
    });
    buttons[7]->setPressedAction([this]
    { 
        
    });
    setLayout();
}