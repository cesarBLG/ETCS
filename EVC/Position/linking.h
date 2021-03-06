/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
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
#include "../Packets/5.h"
#include <list>
struct bg_id
{
    int NID_C;
    int NID_BG;
    bool operator==(const bg_id &o) const
    {
        return NID_C == o.NID_C && NID_BG == o.NID_BG;
    }
};
struct link_data
{
    bg_id nid_bg;
    distance dist;
    double locacc;
    bool reverse_dir;
    int reaction;
    distance max()
    {
        return dist+locacc;
    }
    distance min()
    {
        return dist-locacc;
    }
};
extern std::list<link_data> linking;
extern bg_id NID_LRBG;
extern double Q_LOCACC_LRBG;
double update_location_reference(bg_id nid_bg, distance group_pos, bool linked=true);
void update_linking(distance start, Linking link, bool infill, bg_id this_bg);
void delete_linking();