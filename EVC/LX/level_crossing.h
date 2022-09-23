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
#include "../Position/distance.h"
#include "../Packets/88.h"
#include "../optional.h"
#include <set>
class level_crossing
{
    public:
    int id;
    distance start;
    double length;
    bool lx_protected;
    double V_LX;
    bool stop;
    double stoplength;
    mutable optional<distance> svl_replaced;
    level_crossing(LevelCrossingInformation lx, distance ref)
    {
        id = lx.NID_LX;
        start = ref+lx.D_LX.get_value(lx.Q_SCALE);
        length = lx.L_LX.get_value(lx.Q_SCALE);
        lx_protected = lx.Q_LXSTATUS == Q_LXSTATUS_t::Protected;
        if (!lx_protected)
        {
            V_LX = lx.V_LX.get_value();
            stop = lx.Q_STOPLX == Q_STOPLX_t::StopRequired;
            if (stop) stoplength = lx.L_STOPLX.get_value(lx.Q_SCALE);
        }
    }
    bool operator<(const level_crossing l) const
    {
        return start<l.start;
    }
};
extern std::set<level_crossing> level_crossings;
void load_lx(LevelCrossingInformation lx, distance ref);
void update_lx();