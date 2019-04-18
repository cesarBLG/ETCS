#include "menu_main.h"
#include "window.h"
#include "level_window.h"
#include "driver_id.h"
#include "running_number.h"
#include <algorithm>
#include "../monitor.h"
#include "../messages/messages.h"
#include "../time.h"
#include "../control/control.h"

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
        if(trn!=0 && driverid!=0)
        {
            exit(this);
            mode = level == 0 ? UN : SR;
            modeAck = true;
            Vperm = 120;
            Vsbi = 125;
            Vebi = 130;
            Vtarget = -1;
        }
    });
    buttons[1]->setPressedAction([this] 
    {
        exit(this);
        right_menu(new driver_window());
    });
    buttons[4]->setPressedAction([this] 
    {
        exit(this);
        right_menu(new level_window());
    });
    buttons[5]->setPressedAction([this] 
    {
        exit(this);
        right_menu(new trn_window());
    });
    buttons[6]->setPressedAction([this]
    {
        if(driverid!=0)
        {
            exit(this);
            mode = SH;
            modeAck = true;
            Vperm = 40;
            Vsbi = 45;
            Vebi = 50;
            Vtarget = -1;
        }
    });
    buttons[7]->setPressedAction([this]
    { 
        exit(this);
        mode = NL;
    });
    setLayout();
}