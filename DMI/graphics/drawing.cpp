/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <ctime>
#include <chrono>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <deque>
#include <fstream>
#include "drawing.h"
#include "display.h"
#include "button.h"
#include "flash.h"
#include "../sound/sound.h"
#include "../messages/messages.h"
#include "../tcp/server.h"

void update_stm_windows();

void present_frame()
{
    update_stm_windows();
    platform->set_color(DarkBlue);
    platform->clear();
    displayETCS();
    platform->present().then(present_frame).detach();
}

void process_input(UiPlatform::InputEvent ev)
{
    std::vector<window *> windows;
    for (auto it = active_windows.begin(); it != active_windows.end(); ++it)
    {
        windows.push_back(*it);
    }
    for (int i = 0; i < windows.size(); i++)
    {
        if (windows[i]->active) windows[i]->event(ev.action != UiPlatform::InputEvent::Action::Release, ev.x, ev.y);
        else windows[i]->event(0, -100, -100);
    }
    platform->on_input_event().then(process_input).detach();
}

void drawing_start()
{
    platform->on_input_event().then(process_input).detach();

    loadBeeps();
    setupFlash();
    present_frame();
}
