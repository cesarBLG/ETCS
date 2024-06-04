/*
 * European Train Control System
 * Copyright (C) 2024  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <optional>
#include "../Position/distance.h"
struct reversing_supervision
{
    double speed;
    double max_distance;
};
extern std::optional<reversing_supervision> rv_supervision;
struct reversing_area
{
    distance start;
    distance end;
};
extern std::optional<reversing_area> rv_area;
extern std::optional<distance> rv_position;
extern std::optional<distance> rv_area_end_original;
extern bool reversing_permitted;
void update_reversing();