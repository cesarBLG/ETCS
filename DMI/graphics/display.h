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
#ifndef _DISPLAY_H
#define _DISPLAY_H
#include <unordered_set>
#include "../window/window.h"
using namespace std;
extern window etcs_default_window;
extern window* default_window;
extern window navigation_bar;
//extern window menu_main;
extern window planning_area;;
extern unordered_set<window*> active_windows;
extern unordered_set<window*> old_windows;
void displayETCS();
#endif