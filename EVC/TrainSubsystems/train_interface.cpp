/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "train_interface.h"
#include "../TrackConditions/track_condition.h"
#include "train_interface.h"
#include "../STM/stm.h"
bool cab_active[2] = {true, false};
bool sl_signal;
bool ps_signal;
bool nl_signal;
bool isolated;
bool failed;
bool SB_command;
bool EB_command;
double brake_pressure;
int reverser_direction;
track_condition_profile_external regenerative_inhibition;
track_condition_profile_external magnetic_inhibition;
track_condition_profile_external eddy_eb_inhibition;
track_condition_profile_external eddy_sb_inhibition;
track_condition_profile_external neutral_section_info;
track_condition_profile_external lower_pantograph_info;
track_condition_profile_external air_tightness_info;
bool regenerative_inhibition_stm;
bool magnetic_inhibition_stm;
bool eddy_eb_inhibition_stm;
bool eddy_sb_inhibition_stm;
bool neutral_section_stm;
bool lower_pantograph_stm;
bool air_tightness_stm;
bool traction_cutoff_active;
bool ep_brake_available=true;
bool eddy_brake_available=true;
bool regenerative_brake_available=true;
bool magnetic_brake_available=true;
bool additional_brake_available;
extern bool TCO;
void update_train_interface()
{
    traction_cutoff_active = TCO;
    if (mode == Mode::SL || mode == Mode::NL || mode == Mode::SN) {
        for (auto kvp : installed_stms) {
            auto *stm = kvp.second;
            if (stm->active()) {
                traction_cutoff_active |= stm->tiu_function.TCO;
                neutral_section_stm = stm->tiu_function.open_circuit_breaker;
                lower_pantograph_stm = stm->tiu_function.lower_pantograph;
                air_tightness_stm = stm->tiu_function.close_air_intake;
                regenerative_inhibition_stm = stm->tiu_function.regenerative_brake_inhibition;
                magnetic_inhibition_stm = stm->tiu_function.magnetic_shoe_inhibition;
                eddy_eb_inhibition_stm = stm->tiu_function.eddy_emergency_brake_inhibition;
                eddy_sb_inhibition_stm = stm->tiu_function.eddy_service_brake_inhibition;
            }
        }
    }
}