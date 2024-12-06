/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "level_crossing.h"
#include "../MA/movement_authority.h"
#include "../Supervision/targets.h"
#include "../Supervision/supervision_targets.h"
std::list<level_crossing> level_crossings;
bool inform_lx = false;
void load_lx(LevelCrossingInformation lxi, distance ref)
{
    level_crossing lx = level_crossing(lxi, ref);
    for (auto it=level_crossings.begin(); it!=level_crossings.end(); ) {
        if (it->id == lx.id) {
            lx.svl_replaced = it->svl_replaced;
            it = level_crossings.erase(it);
        } else {
            ++it;
        }
    }
    if (lx.start.max < d_maxsafefront(lx.start))
        lx.svl_replaced = speed_restriction(lx.V_LX, distance::from_odometer(d_estfront_dir[odometer_orientation == -1]), lx.start+lx.length, false);
    level_crossings.push_back(lx);
}
void update_lx()
{
    inform_lx = false;
    for (auto it=level_crossings.begin(); it!=level_crossings.end(); ++it) {
        if (!it->lx_protected) {
            if (it->svl_replaced && d_minsafefront(it->start) < it->start.min+it->length) inform_lx = true;
            if (!it->svl_replaced && V_est == 0 && it->stop && d_estfront > it->start.est-it->stoplength) {
                it->svl_replaced = speed_restriction(it->V_LX, it->start-(it->start.max-d_estfront), it->start+it->length, false);
                calculate_SvL();
            }
            if (EoA && EoA->est == it->start.est && !it->svl_replaced) {
                const std::list<std::shared_ptr<target>> &supervised_targets = get_supervised_targets();
                std::shared_ptr<target> tEoA, tSvL;
                for (auto &it : supervised_targets) {
                    if (it->type == target_class::EoA)
                        tEoA = it;
                    if (it->type == target_class::SvL)
                        tSvL = it;
                }
                if (MRDT && (*MRDT == *tEoA || *MRDT == *tSvL)) inform_lx = true;
                if (it->stop)
                    continue;
                tEoA->calculate_curves(it->V_LX, 0, f41(it->V_LX));
                tSvL->calculate_curves(it->V_LX, 0, f41(it->V_LX));
                bool c1 = tEoA->d_P <= d_estfront;
                bool c2 = tSvL->d_P <= d_maxsafefront(tSvL->get_target_position());
                if (c1 || c2) {
                    distance start = it->start;
                    if (c1)
                        start -= tEoA->get_target_position() - tEoA->d_P;
                    else
                        start -= tSvL->get_target_position() - tSvL->d_P;
                    it->svl_replaced = speed_restriction(it->V_LX, start, it->start+it->length, false);
                    calculate_SvL();
                }
            }
        }
    }
}