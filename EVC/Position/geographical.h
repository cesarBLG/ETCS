/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "distance.h"
#include "linking.h"
#include "../optional.h"
#include "../Packets/79.h"
struct geographical_position
{
    bg_id id;
    bool forwards;
    float initial_val;
    float start_offset;
    optional<distance> bg_ref;
    float get_position(distance d)
    {
        if (!bg_ref)
            return -1;
        float travelled = d-*bg_ref+start_offset;
        if (forwards)
            return initial_val + travelled;
        else
            return initial_val - travelled;
    }
};
extern optional<geographical_position> valid_geo_reference;
void handle_geographical_position(GeographicalPosition p, bg_id this_bg);
void geographical_position_handle_bg_passed(bg_id id, distance ref, bool reverse);
void update_geographical_position();