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
#include "data_entry.h"
#include "window.h"
#include "keyboard.h"
#include <algorithm>
#include "../graphics/button.h"
#include "../graphics/display.h"
#include "../tcp/server.h"
input_window::input_window(string title, int nfields, bool full) : subwindow(title, full, (nfields-1)/4 + 1), confirmation_label(330, 40), button_yes(gettext("Yes"),330,40), 
    nfields(nfields)
{
    for(int i=0; i<12; i++)
    {
        empty_button[i] = new Button(102,50);
        empty_button[i]->showBorder = false;
        buttons[i] = empty_button[i];
    }
    if(fullscreen)
    {
        button_yes.setBackgroundColor(DarkGrey);
        button_yes.setForegroundColor(Black);
        button_yes.disabledColor = Black;
        button_yes.setPressedAction([this, nfields]
        {
            bool valid = true;
            for(int i=0; i<nfields; i++)
            {
                if (!inputs[i]->isAccepted())
                    valid = false;
            }
            if (valid) {
                sendInformation();
            }
        });
        button_yes.showBorder = false;
        button_yes.touch_up = 50;
    }
    confirmation_label.addText(title + gettext(" entry complete?"), 0, 0, 12, White);
}
void input_window::create()
{
    setLayout();
    if(!fullscreen)
    {
        inputs[0]->data_comp->setPressedAction([this]
        {
            if (inputs[0]->data == "") return;
            inputs[0]->setAccepted(true);
            sendInformation();
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
                    if (inputs[cursor]->techcross_invalid || inputs[cursor]->techrange_invalid || inputs[cursor]->data == "") return;
                    json j = R"({"DriverSelection":"ValidateEntryField"})"_json;
                    j["ValidateEntryField"]["Label"] = inputs[i]->label;
                    j["ValidateEntryField"]["Value"] = inputs[i]->data;
                    j["ValidateEntryField"]["SkipOperationalCheck"] = inputs[i]->operatrange_invalid;
                    j["WindowTitle"] = title;
                    write_command("json", j.dump());
                }
                else
                {
                    if (inputs[cursor]->techcross_invalid || inputs[cursor]->techrange_invalid || inputs[cursor]->operatrange_invalid) return;
                    cursor = i;
                    updatePage(cursor/4+1);
                }
            });
        }
    }
}
void input_window::setLayout()
{   
    clearLayout();
    subwindow::setLayout();
    if (cursor/4 + 1 != current_page) cursor = (current_page-1)*4;
    for(int i=0; i<nfields; i++)
    {
        if(i!=cursor) inputs[i]->setSelected(false);
    }
    inputs[cursor]->setSelected(true);
    if(!fullscreen)
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
            if (!inputs[i]->show_echo) continue;
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
void input_window::fieldCheckResult(json &j)
{
    for (int i=0; i<nfields; i++) {
        if (inputs[i]->label != j["Label"]) continue;
        inputs[i]->techrange_invalid = !j["TechnicalRange"];
        inputs[i]->techresol_invalid = !j["TechnicalResolution"];
        inputs[i]->operatrange_invalid = !j["OperationalRange"];
        if (!inputs[i]->techrange_invalid && !inputs[i]->techresol_invalid && !inputs[i]->operatrange_invalid)
        {
            inputs[i]->setAccepted(true);
            if (i+1<nfields) cursor++;
            else cursor = 0;
            updatePage(cursor/4+1);
            bool allaccepted = true;
            for (int j=0; j<nfields; j++) {
                if (!inputs[j]->isAccepted())
                    allaccepted = false;
            }
            if (allaccepted)
            {
                button_yes.setBackgroundColor(Grey);
                button_yes.setEnabled(true);
                button_yes.delayType = false;
            }
        }
        else
        {
            inputs[i]->keybd_data = "";
            inputs[i]->updateText();
            for (int j=0; j<nfields; j++)
            {
                if (i!=j) inputs[j]->data_comp->setEnabled(false);
                else if (!inputs[i]->techrange_invalid && !inputs[i]->techresol_invalid) inputs[j]->data_comp->delayType = true;
                else inputs[j]->data_comp->setEnabled(false);
            }
        }
    }   
}
void input_window::crossResult(json &j)
{
    bool tech = false;
    if (tech)
    {
        button_yes.setEnabled(false);
        button_yes.setBackgroundColor(DarkGrey);
    }
    else
    {
        button_yes.delayType = true;
    }
}
void input_window::inputChanged(input_data *input)
{
    for (int i=0; i<nfields; i++)
    {
        inputs[i]->data_comp->delayType = false;
        inputs[i]->data_comp->setEnabled(true);
    }
    button_yes.setEnabled(false);
    button_yes.setBackgroundColor(DarkGrey);
}
void input_window::sendInformation()
{
    json j = R"({"DriverSelection":"ValidateDataEntry"})"_json;
    j["WindowTitle"] = title;
    for (auto i : inputs)
    {
        j["DataInputResult"][i.second->label] = i.second->data_accepted;
    }
    j["SkipOperationalCheck"] = button_yes.delayType;
    write_command("json", j.dump());
}
void input_window::buildFrom(json &j)
{
    for (int i=0; i<nfields; i++) {
        json &input = j["Inputs"][i];
        inputs[i] = new input_data(input["Label"], input["Label"] != "");
        inputs[i]->window = this;
        inputs[i]->keys = getKeyboard(input["Keyboard"], inputs[i]);
        if (input.contains("AcceptedValue"))
        {
            inputs[i]->data = input["AcceptedValue"].get<std::string>();
            inputs[i]->setAccepted(true);
        }
    }
    create();
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
