/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "menu.h"
#include "platform_runtime.h"
menu::menu(std::string title) : subwindow(title), more("symbols/Navigation/NA_23.bmp", softkeys ? 64 : 153, 50)
{
    hourGlass = new Component(264, 20);
}
menu::~menu()
{
    for(auto *button : buttons)
    {
        if (button != nullptr) delete button;
    }
    for(auto *label : labels)
    {
        if (label != nullptr) delete label;
    }
    delete hourGlass;
}
void menu::setHourGlass(bool show)
{
    if (show)
    {
        if (!hourGlass->graphics.empty()) return;
        int64_t t = platform->get_timer();
        hourGlass->setDisplayFunction([this,t] {

            int d = platform->get_timer() - t;
            ((texture*)hourGlass->graphics[0])->x = 264/2+d*26/1000%254;
        });
        hourGlass->addImage("symbols/Status/ST_05.bmp",-264/2);
    }
    else
    {
        hourGlass->clear();
        hourGlass->setDisplayFunction(nullptr);
    }
}
void menu::buildMenu()
{
    buttons.resize(entries.rbegin()->first+1);
    labels.resize(entries.rbegin()->first+1);
    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
        if (softkeys)
        {
            std::string id = std::to_string(it->first+1);
            buttons[it->first] = new TextButton(id, 64, 50, it->second.pressed, 16);
            labels[it->first] = new Component(266, 24);
            labels[it->first]->addText(id + " - " + it->second.label, 15, 0, 12, White, LEFT);
        }
        else if (it->second.icon == "")
        {
            buttons[it->first] = new TextButton(it->second.label, 153, 50, it->second.pressed);
        }
        else
        {
            buttons[it->first] = new IconButton(it->second.icon, 153, 50, it->second.pressed);
        }
        buttons[it->first]->delayType = it->second.delay;
    }
    if (buttons.size() > softkeys ? 10 : 14)
    {
        more.setPressedAction([this]()
        {
            ++keypage;
            if (keypage * (softkeys ? 9 : 13) > buttons.size()) keypage = 0;
            setLayout();
        });
    }
    setLayout();
}
void menu::setLayout()
{
    clearLayout();
    subwindow::setLayout();
    for (int i=0; i<(softkeys ? (buttons.size() > 10 ? 8 : 9) : (buttons.size() > 14 ? 12 : 13)); i++)
    {
        int id = keypage*(softkeys ? 9 : 13)+i;
        if (id >= buttons.size()) break;
        if (softkeys)
        {
            if (buttons[id] != nullptr) addToLayout(buttons[id], new RelativeAlignment(nullptr, 64*i, 430, 0));
            if (labels[id] != nullptr) addToLayout(labels[id], new RelativeAlignment(nullptr, 334, 100+24*i, 0));
        }
        else
        {
            if (buttons[id] != nullptr) addToLayout(buttons[id], new RelativeAlignment(nullptr, 334 + 153 * (i%2), 45 + 50 * (i/2)));
        }
    }
    if (softkeys)
    {
        extern Component ackButton;
        addToLayout(&ackButton, new RelativeAlignment(nullptr, 600, 348, 0));
        if (buttons.size() > 10) addToLayout(&more, new RelativeAlignment(nullptr, 576, 430));
    }
    else
    {
        if (buttons.size() > 14) addToLayout(&more, new RelativeAlignment(nullptr, 487, 245));
    }
    addToLayout(hourGlass, new RelativeAlignment(&title_bar, 10+42, 10, 0));
}