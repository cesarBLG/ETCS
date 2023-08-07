/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "data_entry.h"
#include "window.h"
#include "keyboards.h"
#include <algorithm>
#include "../graphics/button.h"
#include "../graphics/display.h"
#include "../tcp/server.h"
input_window::input_window(std::string title, int nfields, bool full) : subwindow(title, full, (nfields-1)/4 + 1), confirmation_label(334, 50), button_yes(get_text("Yes"),334,50),
    softYes(get_text("Yes"), 64, 50), up_button("symbols/Navigation/NA_13.bmp", 40, 64, nullptr, "symbols/Navigation/NA_15.bmp"), down_button("symbols/Navigation/NA_14.bmp", 40, 64, nullptr, "symbols/Navigation/NA_16.bmp"),
    nfields(nfields)
{
    for(int i=0; i<12; i++)
    {
        empty_button[i] = new Button(softkeys ? 64 : 102, 50);
        empty_button[i]->showBorder = false;
        buttons[i] = empty_button[i];
    }
    if(fullscreen)
    {
        button_yes.setBackgroundColor(DarkGrey);
        button_yes.setForegroundColor(Black);
        button_yes.disabledColor = Black;
        auto act = [this, nfields]
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
        };
        button_yes.setEnabled(false);
        softYes.setEnabled(false);
        button_yes.setPressedAction(act);
        softYes.setPressedAction(act);
        button_yes.showBorder = false;
        button_yes.touch_up = 50;
    }
    up_button.setPressedAction([this, nfields] {
        if (cursor != -1 && (inputs[cursor]->techcross_invalid || inputs[cursor]->techrange_invalid || inputs[cursor]->operatrange_invalid)) return;
        if (cursor > (softkeys ? -1 : 0)) cursor--;
        else cursor = nfields - 1;
        updatePage(cursor/4+1);
    });
    down_button.setPressedAction([this, nfields] {
        if (cursor != -1 && (inputs[cursor]->techcross_invalid || inputs[cursor]->techrange_invalid || inputs[cursor]->operatrange_invalid)) return;
        if (cursor == nfields - 1) cursor = softkeys ? -1 : 0;
        else cursor++;
        updatePage(cursor/4+1);
    });
    confirmation_label.addText(title + get_text(" entry complete?"), 0, 0, 12, White);
}
void input_window::create()
{
    setLayout();
    if(!fullscreen)
    {
        auto act = [this]
        {
            if (inputs[0]->data == "") return;
            inputs[0]->setAccepted(true);
            sendInformation();
        };
        inputs[0]->data_comp->setPressedAction(act);
        inputs[0]->enter_button.setPressedAction(act);
    }
    else 
    {
        for(int i=0; i<nfields; i++)
        {
            auto act = [this, i]
            {
                if (i == cursor)
                {
                    if (inputs[cursor]->techcross_invalid || inputs[cursor]->techrange_invalid || inputs[cursor]->data == "") return;
                    json j = R"({"DriverSelection":"ValidateEntryField"})"_json;
                    j["ValidateEntryField"]["Label"] = inputs[i]->label;
                    j["ValidateEntryField"]["Value"] = inputs[i]->data;
                    j["ValidateEntryField"]["SkipOperationalCheck"] = inputs[i]->data_comp->delayType;
                    j["WindowTitle"] = title;
                    write_command("json", j.dump());
                    for (int j=0; j<nfields; j++)
                    {
                        inputs[j]->data_comp->setEnabled(false);
                        inputs[j]->enter_button.setEnabled(false);
                    }
                    next_button.setEnabled(false);
                    prev_button.setEnabled(false);
                    up_button.setEnabled(false);
                    down_button.setEnabled(false);
                    button_yes.setEnabled(false);
                    button_yes.setBackgroundColor(DarkGrey);
                    softYes.setEnabled(false);
                }
                else
                {
                    if (cursor != -1 && (inputs[cursor]->techcross_invalid || inputs[cursor]->techrange_invalid || inputs[cursor]->operatrange_invalid)) return;
                    cursor = i;
                    updatePage(cursor/4+1);
                }
            };
            inputs[i]->data_comp->setPressedAction(act);
            inputs[i]->enter_button.setPressedAction(act);
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
    if (cursor != -1) inputs[cursor]->setSelected(true);
    if(!fullscreen)
    {
        addToLayout(inputs[0]->data_comp, new RelativeAlignment(nullptr, 334, softkeys ? 50 : 65, 0));
    }
    else
    {
        addToLayout(&confirmation_label, new RelativeAlignment(nullptr, 0, softkeys ? 330 : 365, 0));
        addToLayout(&button_yes, new ConsecutiveAlignment(&confirmation_label, DOWN, 0));
        if (cursor == -1) addToLayout(&softYes, new RelativeAlignment(nullptr, 128, 430, 0));
        for(int i=4*(cursor/4); i<nfields && i<(4*(cursor/4+1)); i++)
        {
            addToLayout(inputs[i]->label_comp, new RelativeAlignment(nullptr, 334, (softkeys ? 0 : 15)+(i%4)*50, 0));
            addToLayout(inputs[i]->data_comp, new ConsecutiveAlignment(inputs[i]->label_comp, RIGHT, 0));
        }
        for(int i=0; i<nfields; i++)
        {
            if (!inputs[i]->show_echo) continue;
            addToLayout(inputs[i]->data_echo, new RelativeAlignment(nullptr, 204, 100+i*16, 0));
            addToLayout(inputs[i]->label_echo, new ConsecutiveAlignment(inputs[i]->data_echo, LEFT, 0));
        }
    }
    if (softkeys && fullscreen)
    {
        addToLayout(&up_button, new RelativeAlignment(nullptr, 600, 220, 0));
        addToLayout(&down_button, new ConsecutiveAlignment(&up_button, DOWN, 0));
    }
    if (cursor != -1)
    {
        for(int i=0; i<12; i++)
        {
            buttons[i] = empty_button[i];
        }
        input_data *input=inputs[cursor];
        int size = input->keybd.keys.size();
        int max = softkeys ? 9 : 11;
        for(int i=0; i<max+1; i++)
        {
            if (i == max && size > max+1) 
            {
                input->more.setPressedAction([this, input, max, size](){
                    input->keypage++;
                    if (size <= max*input->keypage) input->keypage = 0;
                    setLayout();
                });
                buttons[max] = &input->more;
                break;
            }
            else if (i+input->keypage*max < size && input->keybd.keys[i+input->keypage*max]!=nullptr)
            {
                int idx = i+input->keypage*max;
                buttons[i] = input->keybd.keys[idx];
                if (softkeys && idx < input->keybd.labels.size()) addToLayout(input->keybd.labels[idx], new RelativeAlignment(nullptr, 334, (fullscreen ? 215 : 165)+24*i, 0));
            }
        }
        if (softkeys)
        {
            addToLayout(&inputs[cursor]->enter_button, new RelativeAlignment(nullptr, 600, 348, 0));
            addToLayout(buttons[0], new RelativeAlignment(nullptr, 0, 430, 0));
            for (int i=1; i<10; i++)
            {
                addToLayout(buttons[i], new ConsecutiveAlignment(buttons[i-1], RIGHT, 0));
            }
            if (input->keybd.del != nullptr) addToLayout(input->keybd.del, new RelativeAlignment(nullptr, 600, 28, 0));
        }
        else
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
    }
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
            else cursor = softkeys ? -1 : 0;
            updatePage(cursor/4+1);
            bool allaccepted = true;
            for (int j=0; j<nfields; j++) {
                inputs[j]->data_comp->setEnabled(true);
                inputs[j]->data_comp->delayType = false;
                inputs[j]->enter_button.setEnabled(true);
                inputs[j]->enter_button.delayType = false;
                if (!inputs[j]->isAccepted())
                    allaccepted = false;
            }
            up_button.setEnabled(true);
            down_button.setEnabled(true);
            if (allaccepted)
            {
                button_yes.setBackgroundColor(Grey);
                button_yes.setEnabled(true);
                softYes.setEnabled(true);
                button_yes.delayType = false;
                softYes.delayType = false;
            }
        }
        else
        {
            inputs[i]->keybd_data = "";
            inputs[i]->updateText();
            if (!inputs[i]->techrange_invalid && !inputs[i]->techresol_invalid)
            {
                inputs[i]->data_comp->delayType = true;
                inputs[i]->data_comp->setEnabled(true);
                inputs[i]->enter_button.delayType = true;
                inputs[i]->enter_button.setEnabled(true);
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
        softYes.setEnabled(false);
    }
    else
    {
        button_yes.delayType = true;
        softYes.delayType = true;
    }
}
void input_window::inputChanged(input_data *input)
{
    for (int i=0; i<nfields; i++)
    {
        inputs[i]->data_comp->delayType = false;
        inputs[i]->data_comp->setEnabled(true);
        inputs[i]->enter_button.delayType = false;
        inputs[i]->enter_button.setEnabled(true);
    }
    next_button.enabled = current_page < page_count;
    prev_button.enabled = current_page > 1;
    up_button.setEnabled(true);
    down_button.setEnabled(true);
    button_yes.setEnabled(false);
    button_yes.setBackgroundColor(DarkGrey);
    softYes.setEnabled(false);
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
    bool allaccepted = true;
    for (int i=0; i<nfields; i++) {
        json &input = j["Inputs"][i];
        inputs[i] = new input_data(input["Label"], input.contains("Echo") ? input["Echo"].get<bool>() : input["Label"] != "");
        inputs[i]->window = this;
        inputs[i]->keybd = getKeyboard(input["Keyboard"], inputs[i]);
        if (input.contains("Value"))
        {
            inputs[i]->prev_data = inputs[i]->data = input["Value"].get<std::string>();
            //inputs[i]->setAccepted(true);
        }
        if (!inputs[i]->isAccepted()) allaccepted = false;
    }
    if (allaccepted)
    {
        button_yes.setBackgroundColor(Grey);
        button_yes.setEnabled(true);
        button_yes.delayType = false;
        softYes.setEnabled(true);
        softYes.delayType = false;
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
