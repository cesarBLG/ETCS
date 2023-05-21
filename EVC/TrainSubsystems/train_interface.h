/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "../optional.h"
extern bool sl_signal;
extern bool ps_signal;
extern bool nl_signal;
extern bool isolated;
extern bool SB_command;
extern double brake_pressure;
extern bool EB_command;
struct track_condition_profile_external
{
    optional<double> start;
    optional<double> end;
};
extern track_condition_profile_external regenerative_inhibition;
extern track_condition_profile_external magnetic_inhibition;
extern track_condition_profile_external eddy_eb_inhibition;
extern track_condition_profile_external eddy_sb_inhibition;
extern track_condition_profile_external neutral_section_info;
extern track_condition_profile_external lower_pantograph_info;
extern track_condition_profile_external air_tightness_info;
extern bool regenerative_inhibition_stm;
extern bool magnetic_inhibition_stm;
extern bool eddy_eb_inhibition_stm;
extern bool eddy_sb_inhibition_stm;
extern bool neutral_section_stm;
extern bool lower_pantograph_stm;
extern bool air_tightness_stm;
extern bool traction_cutoff_status;
extern bool cab_active[2];
extern int reverser_direction;
extern optional<double> set_speed;
extern bool additional_brake_active;
void update_train_interface();