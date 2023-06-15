/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "../Position/distance.h"
#include "../optional.h"
#include <map>
extern std::map<int, optional<distance>> emergency_stops;
void handle_unconditional_emergency_stop(int id);
int handle_conditional_emergency_stop(int id, distance location, distance minsafe);
void revoke_emergency_stop(int id);