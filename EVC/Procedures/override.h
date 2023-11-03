/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <optional>
#include "../Position/distance.h"
extern bool overrideProcedure;
extern std::optional<distance> formerEoA;
void start_override();
void update_override();
void override_stopsr();
void override_stopsh();