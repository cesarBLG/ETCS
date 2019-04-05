#include "data_entry.h"
#include "menu_main.h"
#include "button.h"
#include "window.h"
#include <algorithm>
#include "display.h"
input_window::input_window(const char *title) : subwindow(title), input_field(204+102,50)
{
    for(int i=0; i<12; i++)
    {
        buttons[i] = nullptr;
    }
    input_field.setBackgroundColor(DarkGrey);
    input_field.setDisplayFunction([this]
    {
        input_field.drawText(data.c_str(),10,0,0,0,12,Grey, LEFT);
    });
    input_field.setPressedAction([this]
    {
        //TODO: validate data
        exit(this);
    });
    addToLayout(&input_field, new RelativeAlignment(nullptr, 334, 65, 0));
}
void input_window::setLayout()
{   
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
input_window::~input_window()
{
    for(int i=0; i<12; i++)
    {
        if(buttons[i]!=nullptr) delete buttons[i];
    }
}
