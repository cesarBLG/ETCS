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
#include "../TrackConditions/track_condition.h"
#include "power.h"
#include "../STM/stm.h"
track_condition_profile_external neutral_section_info;
track_condition_profile_external lower_pantograph_info;
track_condition_profile_external air_tightness_info;
bool main_power_on_available=true;
bool main_power_off_available=true;
bool raise_pantograph_available=false;
bool lower_pantograph_available=true;
bool traction_cutoff_status;
bool traction_cutoff_available=true;
int reverser_direction;
extern bool TCO;
void update_power_status()
{
    traction_cutoff_status = !TCO;
    if (mode == Mode::SL || mode == Mode::NL || mode == Mode::SN) {
        for (auto kvp : installed_stms) {
            auto *stm = kvp.second;
            if (stm->active()) {
                traction_cutoff_status &= !stm->commands.TCO;
            }
        }
    }
}