/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <string>
#include "common.h"
const char Mode_str[][3] = {"FS","LS","OS","SR","SH","UN","PS","SL","SB","TR","PT","SF","IS","NP","NL","SN","RV"};
extern Level level;
extern int nid_ntc;
extern bool level_valid;
extern std::string driver_id;
extern bool driver_id_valid;
extern int train_running_number;
extern bool train_running_number_valid;
extern Mode mode;
extern double V_est;
extern double V_ura;
extern double A_est;
extern double V_perm;
extern double V_target;
extern double V_sbi;
extern double D_target;
extern double V_release;
extern double T_bs1;
extern double T_bs2;
double calc_ceiling_limit();
void update_supervision();
double calculate_V_release();
inline double f41(double speed)
{
    return speed * 0.01;
}