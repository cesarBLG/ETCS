/*
 * European Train Control System
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
#include "../graphics/component.h"
#include "../monitor.h"
#include "../tcp/server.h"
#include "../sound/sound.h"
#include "acks.h"
void displayBrake();
Component c9(54, 25, displayBrake);
static bool prevB;
void displayBrake()
{
    bool B = EB || SB;
    if (brakeAck) c9.setAck([](){write_command("json",R"({"DriverSelection":"BrakeAcknowledge"})");});
    else c9.setAck(nullptr);
    if(prevB == B) return;
    prevB = B;
    c9.clear();
    if(B) c9.addImage("symbols/Status/ST_01.bmp");
}