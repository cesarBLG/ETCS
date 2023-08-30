/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
extern uint32_t evc_peer;
void displayETCS()
{
    updateAcks();
    if (!evc_peer) return;
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