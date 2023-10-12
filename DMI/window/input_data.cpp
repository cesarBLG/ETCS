/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "input_data.h"
#include "data_entry.h"
#include "../graphics/flash.h"
#include "keyboard.h"
#include "platform_runtime.h"
input_data::input_data(std::string label_text, bool echo) : label(label_text), show_echo(echo), data_get([this] {return getData();}), 
data_set([this](std::string s){setData(s);}), more("symbols/Navigation/NA_23.bmp", softkeys ? 64 : 102, 50), enter_button("symbols/Navigation/NA_20.bmp", 40, 82, nullptr, "symbols/Navigation/NA_20.bmp")
{
    holdcursor = {0};
    if(label!="")
    {
        label_comp = new Component(softkeys ? 164 : 204,50);
        data_comp = new Button(102,50);
        if (show_echo)
        {
            label_echo = new Component(100,16);
            data_echo = new Component(100,16);
        }
    }
    else data_comp = new Button((softkeys ? 164 : 204)+102,50);
#if SIMRAIL
    data_tex = data_comp->getText(getFormattedData(data), 10, 0, 12, selected ? Black : (accepted ? White : DarkGrey), LEFT);
#else
    data_tex = data_comp->getText(getFormattedData(data), 10, 0, 12, selected ? Black : (accepted ? White : Grey), LEFT);
#endif
    data_comp->add(data_tex);
    data_comp->showBorder = false;
    data_comp->setDisplayFunction([this]
    {
        data_comp->setBorder(MediumGrey);
        if (selected && flash_state%2)
        {
            float curx = data_tex->width+data_tex->offx;
            float cury = 32;
            std::string text = getFormattedData(data);
            if (text.find('\n') != std::string::npos)
            {
                auto font = platform->load_font(12, false, get_language());
                if (font == nullptr)
                    return;
                curx = data_tex->offx + font->calc_size(text.substr(text.find('\n')+1)).first;
                cury = 42;
            }
            int64_t now = platform->get_timer();
            if (keybd_data.empty()) curx = data_tex->offx;
            else if (now - holdcursor<2000) curx-=9;
            data_comp->drawRectangle(curx, cury, 9, 1, Black);
        }
    });
    if(label!="")
    {
        label_comp->setBackgroundColor(DarkGrey);
        label_comp->addBorder(MediumGrey);
        label_comp->addText(label.c_str(), 10, 0, 12, Grey, RIGHT);
        if (show_echo)
        {
            label_echo->addText(label, 5, 0, 12, White, RIGHT);
            data_echo->addText(getFormattedData(prev_data), 4, 0, 12, White, LEFT);
        }
    }
}
void input_data::setData(std::string s)
{
    if (window != nullptr) window->inputChanged(this);
    techcross_invalid = techrange_invalid = techresol_invalid = operatcross_invalid = operatrange_invalid = false;
    data = s;
    keybd_data = s;
    prev_data = "";
    setAccepted(false);
}
void input_data::setSelected(bool val)
{
    if (selected != val) 
    {
        selected = val;
        data = prev_data;
        if (selected) keybd_data = "";
    }
    updateText();
}
void input_data::setAccepted(bool val)
{
    if (accepted != val)
    {
        accepted = val;
        if (accepted) data_accepted = data;
        else data_accepted = "";
        prev_data = data_accepted;
        if (!selected) data = prev_data;
        if (!accepted) techresol_invalid = techrange_invalid = false;
    }
    updateText();
}
void input_data::updateText()
{
    data_comp->setBackgroundColor(selected ? MediumGrey : DarkGrey);
    data_comp->clear();
#if SIMRAIL
    data_tex = data_comp->getText(getFormattedData(data), 10, 0, 12, selected ? Black : (accepted ? White : DarkGrey), LEFT);
#else
    data_tex = data_comp->getText(getFormattedData(data), 10, 0, 12, selected ? Black : (accepted ? White : Grey), LEFT);
#endif
    data_comp->add(data_tex);
    if(label!="" && show_echo)
    {
        data_echo->clear();
        if (techrange_invalid || techresol_invalid)
            data_echo->addText("++++", 4, 0, 12, Red, LEFT);
        else if (techcross_invalid)
            data_echo->addText("????", 4, 0, 12, Red, LEFT);
        else if (operatrange_invalid)
            data_echo->addText("++++", 4, 0, 12, Yellow, LEFT);
        else if (operatcross_invalid)
            data_echo->addText("????", 4, 0, 12, Yellow, LEFT);
        else
#if SIMRAIL
            data_echo->addText(getFormattedData(data), 4, 0, 12, accepted ? White : DarkGrey, LEFT);
#else
            data_echo->addText(getFormattedData(data), 4, 0, 12, accepted ? White : Grey, LEFT);
#endif
    }
}
input_data::~input_data()
{
    for(auto *comp : keybd.keys)
    {
        if (comp != nullptr) delete comp;
    }
    for(auto *comp : keybd.labels)
    {
        if (comp != nullptr) delete comp;
    }
    if (keybd.del != nullptr) delete keybd.del;
    if(label_comp!=nullptr) delete label_comp;
    if(data_comp!=nullptr) delete data_comp;
    if(label_echo!=nullptr) delete label_echo;
    if(data_echo!=nullptr) delete data_echo;
}