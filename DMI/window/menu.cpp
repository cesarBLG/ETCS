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
menu::menu(std::string title) : subwindow(title)
{
    hourGlass = new Component(264, 20);
}
menu::~menu()
{
    for(int i=0; i<10; i++)
    {
        if (buttons[i] != nullptr) delete buttons[i];
    }
    for(int i=0; i<10; i++)
    {
        if (labels[i] != nullptr) delete labels[i];
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
    for (int i=0; i<10; i++)
    {
        auto it = entries.find(i);
        if (it != entries.end())
        {
            if (softkeys)
            {
                std::string id = std::to_string(i+1);
                buttons[i] = new TextButton(id, 64, 50, it->second.pressed, 16);
                labels[i] = new Component(266, 24);
                labels[i]->addText(id + " - " + it->second.label, 15, 0, 12, White, LEFT);
            }
            else if (it->second.icon == "")
            {
                buttons[i] = new TextButton(it->second.label, 153, 50, it->second.pressed);
            }
            else
            {
                buttons[i] = new IconButton(it->second.icon, 153, 50, it->second.pressed);
            }
            buttons[i]->delayType = it->second.delay;
        }
        else
        {
            buttons[i] = new Button(softkeys ? 64 : 153,50);
            buttons[i]->showBorder = false;
            labels[i] = nullptr;
        }
    }
    setLayout();
}
void menu::setLayout()
{
    subwindow::setLayout();
    if (softkeys)
    {
        addToLayout(buttons[0], new RelativeAlignment(nullptr, 0, 430, 0));
        for (int i=1; i<10; i++)
        {
            addToLayout(buttons[i], new ConsecutiveAlignment(buttons[i-1], RIGHT, 0));
        }
        for (int i=0; i<10; i++)
        {
            if (labels[i] != nullptr) addToLayout(labels[i], new RelativeAlignment(nullptr, 334, 100+24*i, 0));
        }
    }
    else
    {
        addToLayout(buttons[0], new RelativeAlignment(nullptr, 334, 45,0));
        addToLayout(buttons[1], new ConsecutiveAlignment(buttons[0],RIGHT,0));
        addToLayout(buttons[2], new ConsecutiveAlignment(buttons[0],DOWN,0));
        addToLayout(buttons[3], new ConsecutiveAlignment(buttons[2],RIGHT,0));
        addToLayout(buttons[4], new ConsecutiveAlignment(buttons[2],DOWN,0));
        addToLayout(buttons[5], new ConsecutiveAlignment(buttons[4],RIGHT,0));
        addToLayout(buttons[6], new ConsecutiveAlignment(buttons[4],DOWN,0));
        addToLayout(buttons[7], new ConsecutiveAlignment(buttons[6],RIGHT,0));
        addToLayout(buttons[8], new ConsecutiveAlignment(buttons[6],DOWN,0));
        addToLayout(buttons[9], new ConsecutiveAlignment(buttons[8],RIGHT,0));
    }
    addToLayout(hourGlass, new RelativeAlignment(&title_bar, 10+42, 10, 0));
}