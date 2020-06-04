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
subwindow::subwindow(string title, bool full) : window(), fullscreen(full), exit_button("symbols/Navigation/NA_11.bmp", 82, 50), title_bar(full ? 334 : 306,24)
{
    title_bar.addText(title,4,0,12,White, full ? RIGHT : LEFT);
    if (full) title_bar.setBackgroundColor(Black);
    exit_button.setPressedAction([this] 
    {
        exit(this);
    });
}
void subwindow::setLayout()
{
    extern Button downArrow;
    if(fullscreen) addToLayout(&title_bar, new RelativeAlignment(nullptr,0,15,0));
    else addToLayout(&title_bar, new RelativeAlignment(nullptr,334,15,0));
    addToLayout(&exit_button, new ConsecutiveAlignment(&downArrow,RIGHT,0));
}