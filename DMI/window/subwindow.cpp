/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "subwindow.h"
#include "../tcp/server.h"
subwindow::subwindow(std::string title, bool full, int npages) : window(), fullscreen(full), title(title), exit_button("symbols/Navigation/NA_11.bmp", 82, 50, nullptr, "symbols/Navigation/NA_12.bmp"), prev_button("symbols/Navigation/NA_18.bmp", 82,50, nullptr, "symbols/Navigation/NA_19.bmp"),
    next_button("symbols/Navigation/NA_17.bmp", 82,50, nullptr, "symbols/Navigation/NA_18.2.bmp"), title_bar(full ? 334 : 306,24), page_count(npages)
{
    title_bar.setBackgroundColor(Black);
    exit_button.setPressedAction([this]
    {
        write_command("json",R"({"DriverSelection":"CloseWindow"})");
    });
    next_button.setPressedAction([this]
    {
        if (current_page < page_count) updatePage(current_page+1);
        setLayout();
    });
    prev_button.setPressedAction([this]
    {
        if (current_page > 1) updatePage(current_page-1);
    });
    next_button.enabled = page_count > 1;
    prev_button.enabled = false;
    updateTitle();
}
void subwindow::updateTitle()
{
    std::string t = title;
    if (page_count > 1) t += " ("+std::to_string(current_page)+"/"+std::to_string(page_count)+")";
    title_bar.clear();
    title_bar.addText(t,4,0,12,White, fullscreen ? RIGHT : LEFT);
}
void subwindow::updatePage(int num)
{
    current_page = num;
    next_button.enabled = current_page < page_count;
    prev_button.enabled = current_page > 1;
    updateTitle();
    setLayout();
}
void subwindow::setLayout()
{
    extern Button downArrow;
    if(fullscreen) addToLayout(&title_bar, new RelativeAlignment(nullptr,0,15,0));
    else addToLayout(&title_bar, new RelativeAlignment(nullptr,334,15,0));
    addToLayout(&exit_button, new ConsecutiveAlignment(&downArrow,RIGHT,0));
    if (page_count > 1)
    {
        addToLayout(&prev_button, new ConsecutiveAlignment(&exit_button, RIGHT, 0));
        addToLayout(&next_button, new ConsecutiveAlignment(&prev_button, RIGHT, 0));
    }
}