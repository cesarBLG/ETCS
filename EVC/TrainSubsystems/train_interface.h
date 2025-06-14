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
#include <map>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
extern bool sl_signal;
extern bool ps_signal;
extern bool nl_signal;
extern bool isolated;
extern bool failed;
extern bool SB_command;
extern double brake_pressure;
extern bool EB_command;
extern bool traction_cutoff_active;
extern bool cab_active[2];
extern int reverser_direction;
extern std::string obu_tr_status;
extern optional<double> set_speed;
extern bool ep_brake_available;
extern bool eddy_brake_available;
extern bool regenerative_brake_available;
extern bool magnetic_brake_available;
extern bool additional_brake_available;
extern bool automatic_close_air_intake;
extern bool automatic_open_air_intake;
extern bool automatic_eddy_inhibition;
extern bool automatic_magnetic_inhibition;
extern bool automatic_regenerative_inhibition;
void update_train_interface();
void handle_tr_inputs(json &j);