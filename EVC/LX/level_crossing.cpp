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
std::set<level_crossing> level_crossings;
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
        lx.svl_replaced = distance::from_odometer(d_estfront_dir[odometer_orientation == -1]);
    level_crossings.insert(lx);
    calculate_SvL();
}
void update_lx()
{
    inform_lx = false;
    for (auto it=level_crossings.begin(); it!=level_crossings.end(); ) {
        if (!it->lx_protected) {
            if (it->svl_replaced && d_minsafefront(it->start) < it->start.min+it->length) inform_lx = true;
            if (!it->svl_replaced && V_est == 0 && it->stop && d_estfront > it->start.est-it->stoplength) {
                it->svl_replaced = distance::from_odometer(d_estfront_dir[odometer_orientation == -1]);
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
                if (it->stop) {
                    ++it;
                    continue;
                }
                tEoA->calculate_curves(it->V_LX);
                tSvL->calculate_curves(it->V_LX);
                bool c1 = tEoA->d_P < d_estfront;
                bool c2 = tSvL->d_P < d_maxsafefront(tSvL->get_target_position());
                if (c1 || c2) {
                    it->svl_replaced = distance::from_odometer(c1 ? d_estfront_dir[odometer_orientation == -1] : d_maxsafefront(tSvL->get_target_position()));
                    calculate_SvL();
                }
            }
        }
        if (d_minsafefront(it->start) > it->start.min+it->length)
            it = level_crossings.erase(it);
        else
            ++it;
    }
}