/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "monitor.h"
#include "graphics/drawing.h"
#include "tcp/server.h"
#include "control/control.h"
#include "platform_runtime.h"

void startWindows();
void initialize_stm_windows();

float platform_size_w = 640.0f, platform_size_h = 480.0f;

void on_platform_ready()
{
    platform->on_quit_request().then([](){
        platform->quit();
    }).detach();

    setSpeeds(0, 0, 0, 0, 0, 0);
    setMonitor(CSM);
    setSupervision(NoS);
    startSocket();
    startWindows();
    initialize_stm_windows();
    drawing_start();
}
