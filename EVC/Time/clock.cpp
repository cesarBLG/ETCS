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
int64_t get_milliseconds()
{
    return platform->get_timer();
}
BasePlatform::DateTime offset_time() {
    BasePlatform::DateTime clock = platform->get_local_time();
    int secs = clock.hour * 3600 + clock.minute * 60 + clock.second + TimeOffset::offset;
    while (secs < 0)
        secs += 86400;
    secs %= 86400;
    clock.hour = secs / 3600;
    clock.minute = (secs - clock.hour * 3600) / 60;
    clock.second = secs - clock.hour * 3600 - clock.minute * 60;
    return clock;
}
