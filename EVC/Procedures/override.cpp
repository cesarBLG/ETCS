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
#include "../Supervision/national_values.h"
#include "../Supervision/speed_profile.h"
#include "../Time/clock.h"
bool overrideProcedure = false;
distance override_end_distance;
int64_t override_end_time;
optional<distance> formerEoA;
bool formerValid = false;
void start_override()
{
    if (V_est <= V_NVALLOWOVTRP) {    
        overrideProcedure = true;
        formerValid = false;
        override_end_distance = d_estfront + D_NVOVTRP;
        override_speed = speed_restriction(V_NVSUPOVTRP, distance(std::numeric_limits<double>::lowest()), override_end_distance, false);
        override_end_time = get_milliseconds() + T_NVOVTRP*1000;
        if (mode == Mode::OS || mode == Mode::LS || mode == Mode::FS) {
            if (EoA)
                formerEoA = EoA;
            else if (LoA)
                formerEoA = LoA->first;
            formerValid = formerEoA && *EoA > d_estfront;
        } else if (mode == Mode::PT || mode == Mode::SB) {
            formerEoA = d_estfront;
        }
        recalculate_MRSP();
        trigger_condition(37);
    }
}
void update_override()
{
    if (overrideProcedure) {
        if (d_estfront > override_end_distance || get_milliseconds() > override_end_time || MA)
            overrideProcedure = false;
        if (formerValid && formerEoA && *formerEoA<d_minsafefront(formerEoA->get_reference())-L_antenna_front)
            overrideProcedure = false;
        if (!overrideProcedure) {
            override_speed = {};
            recalculate_MRSP();
        }
    }
}