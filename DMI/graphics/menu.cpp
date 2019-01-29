#include "menu.h"
menu::menu() : exit_button("symbols/Navigation/NA_11.bmp", 82, 50)
{
    extern Button downArrow;
    addToLayout(&exit_button, new ConsecutiveAlignment(&downArrow,RIGHT,0));
    exit_button.setPressedAction([this] 
    {
        active_windows.insert(&navigation_bar);
        active_windows.insert(&PASP);
        active_windows.erase(this);
    });
}
menu::~menu()
{
    for(int i=0; i<8; i++)
    {
        delete buttons[i];
    }
}
void menu::setLayout()
{
    addToLayout(buttons[0], new ConsecutiveAlignment(buttons[1],LEFT,0));
    addToLayout(buttons[1], new RelativeAlignment(nullptr, 490, 45,0));
    addToLayout(buttons[2], new ConsecutiveAlignment(buttons[0],DOWN,0));
    addToLayout(buttons[3], new ConsecutiveAlignment(buttons[2],DOWN,0));
    addToLayout(buttons[4], new ConsecutiveAlignment(buttons[3],RIGHT,0));
    addToLayout(buttons[5], new ConsecutiveAlignment(buttons[3],DOWN,0));
    addToLayout(buttons[6], new ConsecutiveAlignment(buttons[1],DOWN,0));
    addToLayout(buttons[7], new ConsecutiveAlignment(buttons[5],RIGHT,0));
}