#include "menu_override.h"
#include "../monitor.h"
menu_override::menu_override() : menu("Override")
{
    buttons[0] = new TextButton("EoA", 153, 50);
    buttons[0]->setPressedAction([this]
    { 
        ovEOA = true;
        exit(this);
    });
    setLayout();
};