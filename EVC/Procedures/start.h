/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "../Supervision/train_data.h"
#include "mode_transition.h"
#include "../Packets/radio.h"
enum som_step
{
    S0,
    S1,
    D2,
    D3,
    D7,
    S2,
    S3,
    S4,
    A29,
    S10,
    S12,
    D12,
    S13,
    D10,
    D11,
    S11,
    S20,
    S21,
    S22,
    S23,
    S24,
    S25,
    A31,
    D31,
    A32,
    D32,
    A33,
    A34,
    D33,
    A35,
    D22,
    A23,
    A24,
    A38,
    A39,
    A40
};
extern som_step som_status;
extern bool som_active;
extern bool ongoing_mission;
void update_SoM();
void start_pressed();