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
#ifndef _DATA_VIEW_H
#define _DATA_VIEW_H
#include "subwindow.h"
#include "../monitor.h"
#include <vector>
class data_view_window : public subwindow
{
    protected:
    std::vector<std::string> data;
    std::vector<Component*> components;
    void setLayout() override
    {
        clearLayout();
        for (int i=0; i<data.size(); i+=2)
        {
            Component *c1 = new Component(100,16);
            Component *c2 = new Component(100,16);
            addToLayout(c1, new RelativeAlignment(nullptr, 320+204, 15+62+i/2*16, 0));
            addToLayout(c2, new ConsecutiveAlignment(c1, LEFT, 0));
            c2->addText(data[i], 5, 0, 12, White, RIGHT);
            c1->addText(data[i+1], 4, 0, 12, White, LEFT);
            components.push_back(c1);
            components.push_back(c2);
        }
        subwindow::setLayout();
    }
    public:
    data_view_window() : subwindow("Data view", false)
    {
        data.push_back("Driver ID");
        data.push_back(driverid_valid ? driverid : "");
        data.push_back("");
        data.push_back("");
        data.push_back("Train Running Number");
        data.push_back(trn_valid && trn != 0 ? to_string(trn) : "");
        data.push_back("");
        data.push_back("");
        data.push_back("Train Length");
        data.push_back(train_data_valid && L_train != 0 ? to_string(L_train) : "");
        setLayout();
    }
    ~data_view_window()
    {
        for (auto c : components)
        {
            delete c;
        }
    }
};
#endif