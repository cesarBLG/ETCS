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
#include <list>
#include "../window/window.h"
extern window etcs_default_window;
extern window* default_window;
extern std::list<window*> active_windows;
void displayETCS();
#endif