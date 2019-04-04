#include "input_window.h"
#include "menu_main.h"
#include "button.h"
#include "window.h"
#include <algorithm>
#include "display.h"
input_window::input_window(const char *name) : subwindow(name)
{
    buttons[0] = new TextButton("1", 102, 50, nullptr);
    buttons[1] = new TextButton("2", 102, 50, nullptr);
    buttons[2] = new TextButton("3", 102, 50, nullptr);
    buttons[3] = new TextButton("4", 102, 50, nullptr);
    buttons[4] = new TextButton("5", 102, 50, nullptr);
    buttons[5] = new TextButton("6", 102, 50, nullptr);
    buttons[6] = new TextButton("6", 102, 50, nullptr);
    buttons[7] = new TextButton("8", 102, 50, nullptr);
    buttons[8] = new TextButton("9", 102, 50, nullptr);
    buttons[9] = new TextButton("DEL", 102, 50, nullptr);
    buttons[10] = new TextButton("0", 102, 50, nullptr);
    buttons[11] = new TextButton(".", 102, 50, nullptr);

    addToLayout(buttons[0], new RelativeAlignment(nullptr, 334, 215,0));
    addToLayout(buttons[1], new ConsecutiveAlignment(buttons[0],RIGHT,0));
    addToLayout(buttons[2], new ConsecutiveAlignment(buttons[1],RIGHT,0));
    addToLayout(buttons[3], new ConsecutiveAlignment(buttons[0],DOWN,0));
    addToLayout(buttons[4], new ConsecutiveAlignment(buttons[3],RIGHT,0));
    addToLayout(buttons[5], new ConsecutiveAlignment(buttons[4],RIGHT,0));
    addToLayout(buttons[6], new ConsecutiveAlignment(buttons[3],DOWN,0));
    addToLayout(buttons[7], new ConsecutiveAlignment(buttons[6],RIGHT,0));
    addToLayout(buttons[8], new ConsecutiveAlignment(buttons[7],RIGHT,0));
    addToLayout(buttons[9], new ConsecutiveAlignment(buttons[6],DOWN,0));
    addToLayout(buttons[10], new ConsecutiveAlignment(buttons[9],RIGHT,0));
    addToLayout(buttons[11], new ConsecutiveAlignment(buttons[10],RIGHT,0));
}
    

    