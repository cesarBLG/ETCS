/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "mode_transition.h"
#include "level_transition.h"
#include "override.h"
#include "train_trip.h"
#include "start.h"
void update_procedures()
{
    update_level_status();
    update_mode_status();
    update_override();
    update_trip();
    update_SoM();
}