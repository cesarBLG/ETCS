#include "menu_override.h"
#include "../monitor.h"
#include "../tcp/server.h"
menu_override::menu_override() : menu("Override")
{
    buttons[0] = new TextButton("EoA", 153, 50);
    buttons[0]->setPressedAction([this]
    {
        write_command("override","");
        exit(this);
    });
    setLayout();
};