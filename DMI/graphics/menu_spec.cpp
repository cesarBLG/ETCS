#include "menu_spec.h"
#include "button.h"
#include "window.h"
#include <algorithm>
#include "display.h"

menu_spec::menu_spec() : menu("Special")
{
    buttons[0] = new TextButton("Adhesion", 153, 50, nullptr);
    buttons[1] = new TextButton("SR Speed/Distance", 153, 50, nullptr);
    buttons[2] = new TextButton("Train integrity", 153, 50, nullptr);
    setLayout();
}