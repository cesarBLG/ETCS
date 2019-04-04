#include "menu_override.h"
#include "button.h"
#include "window.h"
#include <algorithm>
#include "display.h"

menu_override::menu_override() : menu("Override")
{
    buttons[0] = new TextButton("EoA", 153, 50, nullptr);
    setLayout();
};