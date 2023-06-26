/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "time.h"
#include "time_etcs.h"
#include "platform_runtime.h"

int TimeOffset::offset;

static BasePlatform::DateTime offset_time() {
    BasePlatform::DateTime clock = platform->get_local_time();
    clock.second += TimeOffset::offset;
    while (clock.second >= 60) {
        clock.second -= 60;
        clock.minute++;
    }
    while (clock.minute >= 60) {
        clock.minute -= 60;
        clock.hour++;
    }
    while (clock.hour >= 24)
        clock.hour -= 24;
    return clock;
}

int getHour()
{
    return offset_time().hour;
}
int getMinute()
{
    return offset_time().minute;
}
int getSecond()
{
    return offset_time().second;
}