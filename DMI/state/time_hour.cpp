/*
 * European Train Control System
 * Copyright (C) 2019  Iván Izquierdo
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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