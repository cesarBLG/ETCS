/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "../DMI/text_message.h"
extern bool brake_acknowledgeable;
extern bool brake_acknowledged;
struct brake_command_information
{
    int reason;
    text_message *msg;
    std::function<bool(brake_command_information &i)> revoke;
};
extern std::list<brake_command_information> brake_conditions;
extern std::list<brake_command_information> emergency_brake_conditions;
void trigger_brake_reason(int reason);