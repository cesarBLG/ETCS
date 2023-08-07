/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <vector>
#include <algorithm>
#include <deque>
#include "drawing.h"
#include "display.h"
#include "button.h"
#include "flash.h"
#include "../sound/sound.h"
#include "../messages/messages.h"
#include "../tcp/server.h"
#include "../softkeys/softkey.h"
#include "platform_runtime.h"

bool softkeys=false;
void update_stm_windows();
UiPlatform::InputEvent last_event = {UiPlatform::InputEvent::Action::Release, 0, 0};
void process_input(UiPlatform::InputEvent ev)
{
    for (auto &w : active_windows)
    {
        if (w->active) w->event(ev.action != UiPlatform::InputEvent::Action::Release, ev.x, ev.y);
        else w->event(0, -100, -100);
    }
}
void present_frame()
{
    process_input(last_event);
    update_stm_windows();
    platform->set_color(DarkBlue);
    platform->clear();
    displayETCS();
    platform->present().then(present_frame).detach();
}
void input_received(UiPlatform::InputEvent ev)
{
    last_event = ev;
    process_input(last_event);
    platform->on_input_event().then(input_received).detach();
}

void drawing_start()
{
    platform->on_input_event().then(input_received).detach();

    setupSoftKeys();
    loadBeeps();
    setupFlash();
    present_frame();
}
