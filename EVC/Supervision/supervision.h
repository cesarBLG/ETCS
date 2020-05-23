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
#pragma once
#include "../Position/distance.h"
enum MonitoringStatus
{
    CSM,
    TSM,
    RSM,
};
enum SupervisionStatus
{
    NoS,
    IndS,
    OvS,
    WaS,
    IntS
};
enum struct Level 
{
    N0,
    N1,
    N2,
    N3,
    NTC,
    Unknown
};
enum struct Mode
{
    FS,
    LS,
    OS,
    SR,
    SH,
    UN,
    PS,
    SL,
    SB,
    TR,
    PT,
    SF,
    IS,
    NP,
    NL,
    SN,
    RV
};
const char Mode_str[][3] = {"FS","LS","OS","SR","SH","UN","PS","SL","SB","TR","PT","SF","IS","NP","NL","SN","RV"};
extern Level level;
extern Mode mode;
extern double V_est;
extern double V_ura;
extern double A_est;
extern double V_perm;
extern double V_sbi;
extern double D_target;
extern double V_release;
extern double T_bs1;
extern double T_bs2;
void update_supervision();
double calculate_V_release();
