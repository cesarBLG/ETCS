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
#include "override.h"
#include "mode_transition.h"
#include "start.h"
#include "../Position/linking.h"
#include "../Supervision/national_values.h"
#include "../Supervision/speed_profile.h"
#include "../Time/clock.h"
#include "../DMI/windows.h"
bool overrideProcedure = false;
distance override_start_distance;
int64_t override_start_time;
optional<distance> formerEoA;
optional<distance> formerSRdist;
void start_override()
{
    if (V_est <= V_NVALLOWOVTRP && (((mode == Mode::FS || mode == Mode::OS || mode == Mode::LS || mode == Mode::SR || mode == Mode::UN || mode == Mode::PT || mode == Mode::SB || mode == Mode::SN) && train_data_valid) || mode == Mode::SH)) {
        if (som_active) {
            lrbgs.clear();
            position_valid = false;
        }
        overrideProcedure = true;
        formerSRdist = {};
        override_start_distance = d_estfront_dir[odometer_orientation == -1];
        override_speed = speed_restriction(V_NVSUPOVTRP, distance(std::numeric_limits<double>::lowest(), 0, 0), distance(std::numeric_limits<double>::max(), 0, 0), false);
        override_start_time = get_milliseconds();
        if (mode == Mode::OS || mode == Mode::LS || mode == Mode::FS) {
            if (EoA)
                formerEoA = EoA;
            else if (LoA)
                formerEoA = LoA->first;
        } else if (mode == Mode::PT || mode == Mode::SB) {
            formerEoA = d_estfront_dir[odometer_orientation == -1];
        } else if (mode == Mode::SR) {
            formerSRdist = SR_dist;
            if (std::isfinite(D_NVSTFF)) {
                SR_dist = d_estfront_dir[odometer_orientation == -1]+D_NVSTFF;
                SR_speed = speed_restriction(V_NVSTFF, distance(std::numeric_limits<double>::lowest(), 0, 0), *SR_dist, false);
            } else {
                SR_dist = {};
                SR_speed = speed_restriction(V_NVSTFF, distance(std::numeric_limits<double>::lowest(), 0, 0), distance(std::numeric_limits<double>::max(), 0, 0), false);
            }
        }
        recalculate_MRSP();
        trigger_condition(37);
    }
}
bool stopsr_received=false;
bool stopsh_received=false;
void update_override()
{
    if (overrideProcedure) {
        if (d_estfront - override_start_distance > D_NVOVTRP || get_milliseconds() - override_start_time >  T_NVOVTRP*1000)
            overrideProcedure = false;
        if (mode != Mode::UN && mode != Mode::SN) {
            if (stopsr_received || stopsh_received ||
            (MA && MA->get_v_main() > 0) ||
            (formerSRdist && *formerSRdist < d_estfront) || 
            (formerEoA && *formerEoA<d_minsafefront(*formerEoA)-L_antenna_front))
                overrideProcedure = false;
        }
        if (!overrideProcedure) {
            if (formerEoA && *formerEoA<d_minsafefront(*formerEoA)-L_antenna_front)
                formerEoA = {};
            override_speed = {};
            recalculate_MRSP();
        }
    }
    stopsr_received = false;
    stopsh_received = false;
}
void override_stopsr()
{
    formerEoA = {};
    stopsr_received = true;
}
void override_stopsh()
{
    stopsh_received = true;
}