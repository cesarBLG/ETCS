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
 */
#ifndef _DATA_VALIDATION_H
#define _DATA_VALIDATION_H
#include "subwindow.h"
#include "input_data.h"
#include "keyboard.h"
class validation_input : public input_data
{
    public:
    validation_input()
    {
        data = get_text("Yes");
        accepted = true;
        keys = getYesNoKeyboard(this);
    }
};
class validation_window : public subwindow
{
    Button* emptybuttons[12];
    Button* buttons[12];
    input_data *confirmation;
    protected:
    std::vector<input_data *> validation_data;
    virtual void setLayout() override
    {
        clearLayout();
        subwindow::setLayout();
        addToLayout(confirmation->data_comp, new RelativeAlignment(nullptr, 334, 15, 0));
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
        for(int i=0; i<validation_data.size(); i++)
        {
            addToLayout(validation_data[i]->data_echo, new RelativeAlignment(nullptr, 204, 100+i*16, 0));
            addToLayout(validation_data[i]->label_echo, new ConsecutiveAlignment(validation_data[i]->data_echo, LEFT, 0));
        }
    }
    public:
    validation_window(std::string title, std::vector<input_data*> data) : subwindow(title, true), validation_data(data)
    {
        confirmation = new validation_input();
        std::vector<Button*> keys = getYesNoKeyboard(confirmation);
        for(int i=0; i<12; i++)
        {
            emptybuttons[i] = new Button(102,50);
            emptybuttons[i]->showBorder = false;
            if (keys[i] == nullptr) buttons[i] = emptybuttons[i];
            else buttons[i]=confirmation->keys[i];
        }
        confirmation->data_comp->setPressedAction([this]
        {
            confirmation->setAccepted(true);
            sendInformation();
        });
        setLayout();
        confirmation->setSelected(true);
        confirmation->setData(get_text("Yes"));
        confirmation->setAccepted(true);
        for (int i=0; i<data.size(); i++)
        {
            data[i]->setAccepted(true);
        }
    }
    virtual void sendInformation();
    virtual ~validation_window()
    {
        delete confirmation;
        for(int i=0; i<12; i++)
        {
            delete emptybuttons[i];
        }
    }
};
#endif