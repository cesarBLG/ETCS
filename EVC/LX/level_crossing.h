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
        return start.max<l.start.max;
    }
};
extern std::set<level_crossing> level_crossings;
void load_lx(LevelCrossingInformation lx, distance ref);
void update_lx();