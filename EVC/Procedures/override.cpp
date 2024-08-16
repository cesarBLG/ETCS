/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
std::optional<distance> override_start_distance;
int64_t override_start_time;
std::optional<distance> formerEoA;
std::optional<distance> formerSRdist;
void start_override()
{
    if (V_est <= V_NVALLOWOVTRP && (((mode == Mode::FS || mode == Mode::OS || mode == Mode::LS || mode == Mode::SR || mode == Mode::UN || mode == Mode::PT || mode == Mode::SB || mode == Mode::SN) && train_data_valid) || mode == Mode::SH)) {
        if (som_active) {
            orbgs.clear();
            solr = {};
            position_valid = false;
        }
        overrideProcedure = true;
        formerSRdist = {};
        override_start_distance = distance::from_odometer(d_estfront_dir[odometer_orientation == -1]);
        override_speed = speed_restriction(V_NVSUPOVTRP, distance::from_odometer(dist_base::min), distance::from_odometer(dist_base::max), false);
        override_start_time = get_milliseconds();
        if (mode == Mode::OS || mode == Mode::LS || mode == Mode::FS) {
            if (MA && MA->EoA_ma)
                formerEoA = *MA->EoA_ma;
            else if (MA && MA->LoA_ma)
                formerEoA = MA->LoA_ma->first;
        } else if (mode == Mode::PT || mode == Mode::SB) {
            formerEoA = distance::from_odometer(d_estfront_dir[odometer_orientation == -1]);
        } else if (mode == Mode::SR) {
            formerSRdist = SR_dist;
            SR_dist_override = {};
            SR_speed_override = {};
            SR_speed = speed_restriction(V_NVSTFF, distance::from_odometer(dist_base::min), distance::from_odometer(dist_base::max), false);
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
        if (d_estfront - override_start_distance->est > D_NVOVTRP || get_milliseconds() - override_start_time >  T_NVOVTRP*1000)
            overrideProcedure = false;
        if (mode != Mode::UN && mode != Mode::SN) {
            if (stopsr_received || stopsh_received ||
            (MA && MA->get_v_main() > 0) ||
            (formerSRdist && formerSRdist->est < d_estfront) ||
            (formerEoA && formerEoA->min < d_minsafefront(*formerEoA) - L_antenna_front))
                overrideProcedure = false;
        }
        if (!overrideProcedure) {
            if (formerEoA && formerEoA->min < d_minsafefront(*formerEoA) - L_antenna_front)
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