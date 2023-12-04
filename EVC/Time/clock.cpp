/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "clock.h"
#include "platform_runtime.h"
#include "../../DMI/time_etcs.h"
bool external_wall_clock;
int64_t get_milliseconds()
{
    return platform->get_timer();
}
void update_clock()
{
    if (!external_wall_clock) {
        auto time = platform->get_local_time();
        WallClockTime::hour = time.hour;
        WallClockTime::minute = time.minute;
        WallClockTime::second = time.second;
    }
}
