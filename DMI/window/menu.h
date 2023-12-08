/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _MENU_H
#define _MENU_H
#include "subwindow.h"
struct text_menu_entry
{
    std::string label;
    std::string icon;
    std::function<void()> pressed;
    bool delay;
};
class menu : public subwindow
{
    protected:
    Component *hourGlass=nullptr;
    std::vector<Button*> buttons;
    std::vector<Component*> labels;
    std::map<int,text_menu_entry> entries;
    IconButton more;
    int keypage=0;
    public:
    menu(std::string title);
    ~menu();
    void buildMenu();
    void setLayout() override;
    void setHourGlass(bool show);
};
#endif