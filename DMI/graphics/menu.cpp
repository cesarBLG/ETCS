#include "menu.h"
menu::menu() : exit_button("symbols/Navigation/NA_11.bmp", 82, 50)
{
    extern Button downArrow;
    addToLayout(&exit_button, new ConsecutiveAlignment(&downArrow,RIGHT,0));
    exit_button.setPressedAction([this] 
    {
        exit(this);
    });
    for(int i=0; i<8; i++)
    {
        buttons[i] = nullptr;
    }
}
menu::~menu()
{
    for(int i=0; i<8; i++)
    {
        if(buttons[i]!=nullptr) delete buttons[i];
    }
}
void menu::setLayout()
{
    addToLayout(buttons[0], new RelativeAlignment(nullptr, 490-153, 45,0));
    addToLayout(buttons[1], new ConsecutiveAlignment(buttons[0],RIGHT,0));
    addToLayout(buttons[2], new ConsecutiveAlignment(buttons[0],DOWN,0));
    addToLayout(buttons[3], new ConsecutiveAlignment(buttons[2],RIGHT,0));
    addToLayout(buttons[4], new ConsecutiveAlignment(buttons[2],DOWN,0));
    addToLayout(buttons[5], new ConsecutiveAlignment(buttons[4],RIGHT,0));
    addToLayout(buttons[6], new ConsecutiveAlignment(buttons[4],DOWN,0));
    addToLayout(buttons[7], new ConsecutiveAlignment(buttons[6],RIGHT,0));
}