/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "platform.h"
extern bool external_wall_clock;
void update_clock();
int64_t get_milliseconds();
struct WallClockTime {
    static int hour;
    static int minute;
    static int second;
};
