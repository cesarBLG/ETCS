/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */#include "data_entry.h"
#include "window.h"
#include <algorithm>
#include "../graphics/button.h"
#include "../graphics/display.h"
#include "data_validation.h"
input_window::input_window(string title, int nfields) : subwindow(title, nfields>1), prev_button("symbols/Navigation/NA_18.bmp", 82,50, nullptr, "symbols/Navigation/NA_19.bmp"),
    next_button("symbols/Navigation/NA_17.bmp", 82,50, nullptr, "symbols/Navigation/NA_18.2.bmp"), confirmation_label(330, 40), button_yes("Yes",330,40), 
    nfields(nfields)
{
    for(int i=0; i<12; i++)
    {
        empty_button[i] = new Button(102,50);
        empty_button[i]->showBorder = false;
        buttons[i] = empty_button[i];
    }
    if(nfields > 1)
    {
        button_yes.setBackgroundColor(DarkGrey);
        button_yes.setForegroundColor(Black);
        button_yes.setPressedAction([this, nfields]
        {
            bool valid = true;
            for(int i=0; i<nfields; i++)
            {
                inputs[i]->validate();
                if (!inputs[i]->isValid())
                    valid = false;
            }
            if (valid) {
                sendInformation();
                exit(this);
            }
        });
        button_yes.showBorder = false;
        button_yes.touch_up = 50;
        prev_button.enabled = false;
        next_button.enabled = false;
    }
    if(nfields > 4)
    {
        next_button.setPressedAction([this, nfields]
        {
            if((cursor/4)<(nfields/4))
            {
                next_button.enabled = false;
                prev_button.enabled = true;
                cursor = 4*(cursor/4+1);
                setLayout();
            }
        });
        prev_button.setPressedAction([this, nfields]
        {
            if(cursor>3)
            {
                next_button.enabled = true;
                prev_button.enabled = false;
                cursor = 4*(cursor/4-1);
                setLayout();
            }
        });
        next_button.enabled = true;
    }
    confirmation_label.addText(string(title) + " entry complete?", 0, 0, 12, White);
}
void input_window::create()
{
    setLayout();
    if(nfields == 1)
    {
        inputs[0]->data_comp->setPressedAction([this]
        {
            inputs[0]->setAccepted(true);
            inputs[0]->validate();
            if (inputs[0]->isValid()) {
                sendInformation();
                exit(this);
            }
        });
    }
    else 
    {
        for(int i=0; i<nfields; i++)
        {
            inputs[i]->data_comp->setPressedAction([this, i]
            {
                if (i == cursor)
                {
                    inputs[cursor]->operatrange_invalid = false;
                    inputs[cursor]->setAccepted(true);
                    if (i+1<nfields) cursor++;
                    else cursor = 0;
                    bool allaccepted = true;
                    for (int j=0; j<nfields; j++) {
                        if (!inputs[j]->isAccepted())
                            allaccepted = false;
                    }
                    if (allaccepted)
                        button_yes.setBackgroundColor(Grey);
                    else
                        button_yes.setBackgroundColor(DarkGrey);
                }
                else cursor = i;
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
        if(i!=cursor) inputs[i]->setSelected(false);
    }
    inputs[cursor]->setSelected(true);
    if(nfields == 1)
    {
        addToLayout(inputs[0]->data_comp, new RelativeAlignment(nullptr, 334, 65, 0));
    }
    else
    {
        if(nfields>4)
        {
            addToLayout(&prev_button, new ConsecutiveAlignment(&exit_button, RIGHT, 0));
            addToLayout(&next_button, new ConsecutiveAlignment(&prev_button, RIGHT, 0));
        }
        addToLayout(&confirmation_label, new RelativeAlignment(nullptr, 0, 330, 0));
        addToLayout(&button_yes, new ConsecutiveAlignment(&confirmation_label, DOWN, 0));
        for(int i=4*(cursor/4); i<nfields && i<(4*(cursor/4+1)); i++)
        {
            addToLayout(inputs[i]->label_comp, new RelativeAlignment(nullptr, 334, 15+(i%4)*50, 0));
            addToLayout(inputs[i]->data_comp, new ConsecutiveAlignment(inputs[i]->label_comp, RIGHT, 0));
        }
        for(int i=0; i<nfields; i++)
        {
            addToLayout(inputs[i]->data_echo, new RelativeAlignment(nullptr, 204, 100+i*16, 0));
            addToLayout(inputs[i]->label_echo, new ConsecutiveAlignment(inputs[i]->data_echo, LEFT, 0));
        }
    }    
    for(int i=0; i<12; i++)
    {
        buttons[i] = empty_button[i];
    }
    input_data *input=inputs[cursor];
    int size = input->keys.size();
    for(int i=0; i<12; i++)
    {
        if (i == 11 && size > 12) 
        {
            input->more.setPressedAction([this, input](){
                input->keypage++;
                if (input->keys.size() <= 11*input->keypage) input->keypage = 0;
                setLayout();
            });
            buttons[11] = &input->more;
            break;
        }
        else if (i+input->keypage*11 < size && input->keys[i+input->keypage*11]!=nullptr) buttons[i] = input->keys[i+input->keypage*11];
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
    for(int i=0; i<12; i++)
    {
        delete empty_button[i];
    }
}
