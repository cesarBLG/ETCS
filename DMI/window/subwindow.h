/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _SUBWINDOW_H
#define _SUBWINDOW_H
#include "window.h"
#include "../graphics/display.h"
#include "../graphics/icon_button.h"
#include "../graphics/text_button.h"
class subwindow : public window
{
    protected:
    std::string title;
    Component title_bar;
    int current_page=1;
    int page_count;
    IconButton prev_button;
    IconButton next_button;
    public:
    IconButton exit_button;
    bool fullscreen;
    subwindow(std::string title, bool full = false, int npages=1);
    virtual void setLayout();
    void updatePage(int newpage);
    void updateTitle();
};
#endif