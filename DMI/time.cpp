/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <ctime>
#include "time.h"
using namespace std;
tm getTime()
{
    time_t t = time(nullptr);
    return *localtime(&t);
}
int getHour()
{
    return getTime().tm_hour;
}
int getMinute()
{
    return getTime().tm_min;
}
int getSecond()
{
    return getTime().tm_sec;
}