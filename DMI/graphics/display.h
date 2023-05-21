/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _DISPLAY_H
#define _DISPLAY_H
#include <unordered_set>
#include "../window/window.h"
extern window etcs_default_window;
extern window* default_window;
extern window navigation_bar;
//extern window menu_main;
extern window planning_area;;
extern std::unordered_set<window*> active_windows;
extern std::unordered_set<window*> old_windows;
void displayETCS();
#endif