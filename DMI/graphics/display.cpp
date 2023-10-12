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
std::list<window*> active_windows;
extern Mode mode;
extern uint32_t evc_peer;
bool useImperialSystem;
bool prevUseImperialSystem;
void displayETCS()
{
    updateAcks();
    if (!evc_peer) return;
    std::vector<std::vector<int>> alreadyDrawn;
    for(auto it=active_windows.rbegin(); it!=active_windows.rend(); ++it)
    {
        window *w = *it;
        w->display(alreadyDrawn);
        alreadyDrawn.insert(alreadyDrawn.end(), w->bounds.begin(), w->bounds.end());
    }
    if (prevUseImperialSystem != useImperialSystem) {
        prevUseImperialSystem = useImperialSystem;
    }
}