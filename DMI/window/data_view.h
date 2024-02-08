/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _DATA_VIEW_H
#define _DATA_VIEW_H
#include "subwindow.h"
#include "../monitor.h"
#include <vector>
class data_view_window : public subwindow
{
    protected:
    std::vector<Component*> components;
    void setLayout() override
    {
        clearLayout();
        int offset = softkeys ? 0 : 15;
        for (int i=(current_page-1)*10; i<components.size()/2 && i<current_page*10; i++)
        {
            if (components[2*i] != nullptr) addToLayout(components[2*i], new RelativeAlignment(nullptr, 320, offset+62+(i%10)*16, 0));
            if (components[2*i+1] != nullptr) addToLayout(components[2*i+1], new RelativeAlignment(nullptr, 320+(softkeys ? 176 : 204), offset+62+(i%10)*16, 0));
        }
        subwindow::setLayout();
    }
    public:
    data_view_window(std::string title, json &fields) : subwindow(title, false, 2)
    {
        for (json &j : fields)
        {
            std::string label = j["Label"];
            std::string value = j.contains("Value") ? j["Value"] : "";
            if (j.contains("Keyboard") && j["Keyboard"]["Type"] != "Dedicated");
            {
                std::string format = "";
                for (int i=0; i<value.size(); i++)
                {
                    if (i>0 && i%8 == 0) format += "\n";
                    else if (i>0 && i%4 == 0 && i != value.size() - 1) format += " ";
                    format += value[i];
                }
                value = format;
            }
            for (int i=0; ; i++)
            {
                int break1 = label.find('\n');
                int break2 = value.find('\n');
                Component *c1 = new Component(softkeys ? 176 : 204, 16);
                Component *c2 = new Component(softkeys ? 104 : 116, 16);
                c1->addText(label.substr(0, break1), 5, 0, 12, White, RIGHT);
                c2->addText(value.substr(0, break2), 5, 0, 12, White, LEFT);
                components.push_back(c1);
                components.push_back(c2);
                if (break1 == std::string::npos && break2 == std::string::npos) break;
                label = break1 >= label.size()-1 ? "" : label.substr(break1+1);
                value = break2 >= value.size()-1 ? "" : value.substr(break2+1);
            }
        }
        page_count = (components.size()/2-1)/10 + 1;
        next_button.enabled = page_count > 1;
        updateTitle();
        setLayout();
    }
    ~data_view_window()
    {
        for (auto c : components)
        {
            if (c != nullptr) delete c;
        }
    }
};
#endif