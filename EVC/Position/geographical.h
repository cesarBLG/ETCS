/*
 * European Train Control System
 * Copyright (C) 2020  César Benito <cesarbema2009@hotmail.com>
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