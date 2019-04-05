#include "menu_main.h"
#include "button.h"
#include "window.h"
#include "level_window.h"
#include "running_number.h"
#include <algorithm>
#include "display.h"

menu_main::menu_main() : menu("Main")
{
    buttons[0] = new TextButton("Start", 153, 50, nullptr);
    buttons[1] = new TextButton("Driver ID", 153, 50, nullptr);
    buttons[2] = new TextButton("Train Data", 153, 50, nullptr);
    buttons[3] = new TextButton("Maintain shunting", 153, 50, nullptr);
    buttons[4] = new TextButton("Level", 153, 50, nullptr);
    buttons[5] = new TextButton("Train running number", 153, 50, nullptr);
    buttons[6] = new TextButton("Shunting", 153, 50, nullptr);
    buttons[7] = new TextButton("Non-Leading", 153, 50, nullptr);
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
    setLayout();
}