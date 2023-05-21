/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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