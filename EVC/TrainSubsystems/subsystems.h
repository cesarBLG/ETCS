/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "train_interface.h"
#include "asc.h"
void handle_brake_command();
void update_train_subsystems()
{
    update_train_interface();
    handle_brake_command();
    update_asc();
}