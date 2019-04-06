#include "menu_settings.h"
#include "button.h"
#include "window.h"
#include <algorithm>
#include "display.h"
#include "text_button.h"
menu_settings::menu_settings() : menu("Settings")
{
    buttons[0] = new IconButton("symbols/Setting/SE_03.bmp", 153, 50);
    buttons[1] = new IconButton("symbols/Setting/SE_02.bmp", 153, 50);
    buttons[2] = new IconButton("symbols/Setting/SE_01.bmp", 153, 50);
    buttons[3] = new TextButton("System version", 153, 50);
    buttons[4] = new TextButton("Set VBC", 153, 50);
    buttons[5] = new TextButton("Remove VBC", 153, 50);
    setLayout();
};