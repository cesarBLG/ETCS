/*
 * European Train Control System
 * Copyright (C) 2019  Iván Izquierdo
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
#include "input_data.h"
#include "../graphics/flash.h"
#include "keyboard.h"
input_data::input_data(string label_text) : label(label_text), data_get([this] {return getData();}), 
data_set([this](string s){setData(s);}), more("symbols/Navigation/NA_23.bmp", 102, 50)
{
    holdcursor = {0};
    if(label!="")
    {
        label_comp = new Component(204,50);
        data_comp = new Component(204,50);
        label_echo = new Component(100,16);
        data_echo = new Component(100,16);
    }
    else data_comp = new Component(204+102,50);
    data_tex = data_comp->getText(data,10,0,12, selected ? Black : (accepted ? White : Grey), LEFT);
    data_comp->add(data_tex);
    data_comp->setDisplayFunction([this]
    {
        data_comp->setBorder(MediumGrey);
        if (selected && flash_state%2)
        {
            float cur = data_tex->width+data_tex->offx;
            time_t now;
            time(&now);
            if (keybd_data.empty()) cur = data_tex->offx;
            else if (difftime(now, holdcursor)<2) cur-=9;
            data_comp->drawLine(cur, 32, cur+9, 32, Black);
        }
    });
    if(label!="")
    {
        label_comp->setBackgroundColor(DarkGrey);
        label_comp->addText(label.c_str(), 10, 0, 12, Grey, RIGHT);
        label_echo->addText(label, 5, 0, 12, White, RIGHT);
        data_echo->addText(prev_data, 4, 0, 12, White, LEFT);
    }
}
void input_data::setData(string s)
{
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
    data_tex = data_comp->getText(data,10,0,12, selected ? Black : (accepted ? White : Grey), LEFT);
    data_comp->add(data_tex);
    if(label!="")
    {
        data_echo->clear();
        if (techrange_invalid || techresol_invalid)
            data_echo->addText("++++", 4, 0, 12, Red, LEFT);
        if (techcross_invalid)
            data_echo->addText("????", 4, 0, 12, Red, LEFT);
        if (operatrange_invalid)
            data_echo->addText("++++", 4, 0, 12, Yellow, LEFT);
        if (operatcross_invalid)
            data_echo->addText("????", 4, 0, 12, Yellow, LEFT);
        data_echo->addText(data, 4, 0, 12, accepted ? White : Grey, LEFT);
    }
}
input_data::~input_data()
{
    for(int i=0; i<keys.size(); i++)
    {
        if (keys[i]!=nullptr) delete keys[i];
    }
    if(label_comp!=nullptr) delete label_comp;
    if(data_comp!=nullptr) delete data_comp;
    if(label_echo!=nullptr) delete label_echo;
    if(data_echo!=nullptr) delete data_echo;
}