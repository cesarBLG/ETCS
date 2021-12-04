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
bool main_power_switch_status=true;
bool main_power_on_available=true;
bool main_power_off_available=true;
bool pantograph_status = true;
bool raise_pantograph_available=false;
bool lower_pantograph_available=true;
bool traction_cutoff_status;
bool traction_cutoff_available=true;
extern bool TCO;
void update_power_status()
{
    traction_cutoff_status = !TCO;
    bool power = true;
    bool panto = true;
    for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
        track_condition tc = *it->get();
        if (tc.condition == TrackConditions::PowerLessSectionSwitchMainPowerSwitch) {
            if (tc.get_distance_to_train() < V_est * 3 && tc.get_end_distance_to_train() > -L_TRAIN) {
                power = false;
            }
        }
        if (tc.condition == TrackConditions::PowerLessSectionLowerPantograph) {
            if (tc.get_distance_to_train() < V_est * 10 && tc.get_end_distance_to_train() > -L_TRAIN) {
                panto = false;
            }
        }
    }
    if (!power && main_power_switch_status)
        main_power_switch_status = false;
    if (power && !main_power_switch_status)
        main_power_switch_status = true;
    if (!panto && pantograph_status)
        pantograph_status = false;
    if (panto && !pantograph_status)
        pantograph_status = true;
}