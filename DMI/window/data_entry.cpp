#include "data_entry.h"
#include "window.h"
#include <algorithm>
#include "../graphics/button.h"
#include "../graphics/display.h"
input_window::input_window(const char *title, int nfields) : subwindow(title, nfields>1), prev_button("symbols/Navigation/NA_18.bmp",82,50),
    next_button("symbols/Navigation/NA_17.bmp",82,50), confirmation_label(330, 40), button_yes("Yes",330,40), 
    nfields(nfields)
{      
    for(int i=0; i<12; i++)
    {
        buttons[i] = nullptr;
    }
    if(nfields > 1)
    {
        button_yes.setBackgroundColor(DarkGrey);
        button_yes.setForegroundColor(Black);
        button_yes.setPressedAction([this, nfields]
        {
            for(int i=0; i<nfields; i++)
            {
                inputs[i]->validate();
            }
            exit(this);
        });
    }
    confirmation_label.setDisplayFunction([this, title]
    {
        confirmation_label.setText((string(title) + " entry complete?").c_str(), 12, White);
    });
}
void input_window::create()
{
    setLayout();
    if(nfields == 1)
    {
        inputs[0]->data_comp->setPressedAction([this]
        {
            inputs[0]->validate();
            exit(this);
        });
    }
    else 
    {
        for(int i=0; i<nfields && i<4; i++)
        {
            inputs[i]->data_comp->setPressedAction([this, i]
            {
                cursor = i;
                setLayout();
            });
        }
    }
}
void input_window::setLayout()
{   
    clearLayout();
    subwindow::setLayout();
    for(int i=0; i<nfields; i++)
    {
        inputs[i]->setSelected(false);
    }
    inputs[cursor]->setSelected(true);
    if(nfields == 1)
    {
        addToLayout(inputs[0]->data_comp, new RelativeAlignment(nullptr, 334, 65, 0));
    }
    else
    {
        addToLayout(&prev_button, new ConsecutiveAlignment(&exit_button, RIGHT, 0));
        addToLayout(&next_button, new ConsecutiveAlignment(&prev_button, RIGHT, 0));
        addToLayout(&confirmation_label, new RelativeAlignment(nullptr, 0, 330, 0));
        addToLayout(&button_yes, new ConsecutiveAlignment(&confirmation_label, DOWN, 0));
        for(int i=0; i<nfields && i<4; i++)
        {
            addToLayout(inputs[i]->label_comp, new RelativeAlignment(nullptr, 334, 15+i*50, 0));
            addToLayout(inputs[i]->data_comp, new ConsecutiveAlignment(inputs[i]->label_comp, RIGHT, 0));
            addToLayout(inputs[i]->data_echo, new RelativeAlignment(nullptr, 204, 100+i*16, 0));
            addToLayout(inputs[i]->label_echo, new ConsecutiveAlignment(inputs[i]->data_echo, LEFT, 0));
        }
    }    
    for(int i=0; i<12; i++)
    {
        buttons[i] = nullptr;
    }
    for(int i=0; i<inputs[cursor]->keys.size(); i++)
    {
        buttons[i] = inputs[cursor]->keys[i];
    }
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
    for(int i=0; i<inputs.size(); i++)
    {
        delete inputs[i];
    }
}
