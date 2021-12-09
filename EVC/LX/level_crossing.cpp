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
#include "level_crossing.h"
#include "../MA/movement_authority.h"
#include "../Supervision/targets.h"
std::set<level_crossing> level_crossings;
void load_lx(LevelCrossingInformation lxi, distance ref)
{
    level_crossing lx = level_crossing(lxi, ref);
    for (auto it=level_crossings.begin(); it!=level_crossings.end(); ) {
        if (it->id == lx.id) it = level_crossings.erase(it);
        else ++it;
    }
    level_crossings.insert(lx);
    calculate_SvL();
}
void update_lx()
{
    for (auto it=level_crossings.begin(); it!=level_crossings.end(); ++it) {
        if (it->lx_protected) continue;
        if (V_est == 0 && it->stop && d_estfront > it->start-it->stoplength) {
            it->svl_replaced = true;
            it->svl_replaced_loc = d_estfront;
            calculate_SvL();
        }
        if (!it->stop && EoA &&  *EoA == it->start) {
            const std::list<target> &supervised_targets = get_supervised_targets();
            const target *tEoA, *tSvL;
            for (auto it = supervised_targets.begin(); it != supervised_targets.end(); ++it) {
                if (it->type == target_class::EoA)
                    tEoA = &*it;
                if (it->type == target_class::SvL)
                    tSvL = &*it;
            }
            tEoA->calculate_curves(it->V_LX);
            tSvL->calculate_curves(it->V_LX);
            bool c1 = tEoA->d_P < d_estfront;
            bool c2 = tSvL->d_P < d_maxsafefront(tSvL->d_P.get_reference());
            if (c1 || c2) {
                it->svl_replaced = true;
                it->svl_replaced_loc = c1 ? d_estfront : d_maxsafefront(tSvL->d_P.get_reference());
                calculate_SvL();
            }
        }
    }
}