/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "time_hour.h"
#include "../time_etcs.h"
int lastsec = -1;
Component time_hour(63,50,timeHour);
void timeHour()
{
    if (lastsec == getSecond()) return;
    lastsec = getSecond();
    time_hour.clear();
    char time[9];
    snprintf(time,9,"%02d:%02d:%02d",getHour(),getMinute(),getSecond());
    time_hour.addText(time,0,0,10,White);
}