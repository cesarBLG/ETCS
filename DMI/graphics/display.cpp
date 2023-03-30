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
#include "display.h"
#include "button.h"
#include "text_button.h"
#include "../window/menu.h"
#include "../monitor.h"
#include "../control/control.h"
#include "../state/acks.h"
using namespace std;
unordered_set<window*> active_windows;
unordered_set<window*> old_windows;
extern Mode mode;
#include <iostream>
void displayETCS()
{
    if (mode == Mode::SL || mode == Mode::NP || mode == Mode::IS || mode == Mode::PS)
    {
        return;
    }
    updateAcks();
    for(auto it=active_windows.begin(); it!=active_windows.end(); ++it)
    {
        window *w = *it;
        if(w->active) w->display();
    }
    for(auto it=old_windows.begin(); it!=old_windows.end(); ++it)
    {
        delete *it;
    }
    old_windows.clear();
}