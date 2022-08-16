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
#include "subwindow.h"
#include "../tcp/server.h"
subwindow::subwindow(string title, bool full, int npages) : window(), fullscreen(full), title(title), exit_button("symbols/Navigation/NA_11.bmp", 82, 50, nullptr, "symbols/Navigation/NA_12.bmp"), prev_button("symbols/Navigation/NA_18.bmp", 82,50, nullptr, "symbols/Navigation/NA_19.bmp"),
    next_button("symbols/Navigation/NA_17.bmp", 82,50, nullptr, "symbols/Navigation/NA_18.2.bmp"), title_bar(full ? 334 : 306,24), page_count(npages)
{
    if (full) title_bar.setBackgroundColor(Black);
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
    if (page_count > 1) t += " ("+to_string(current_page)+"/"+to_string(page_count)+")";
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