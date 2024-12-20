/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "power.h"
bool automatic_close_power_switch=true;
bool automatic_open_power_switch=true;
bool automatic_raise_pantograph=false;
bool automatic_lower_pantograph=false;
bool automatic_traction_system_change=false;
bool traction_cutoff_implemented=true;
double T_open_mps = 11;
double D_open_mps;
double T_lower_pantograph = 20;
double D_lower_pantograph;
double T_brake_inhibit;
double D_brake_inhibit;
double T_air_tightness=11;
double D_air_tightness;
double T_traction_system_change = 20;
double D_traction_system_change;
double T_current_consumption_change = 5;
double D_current_consumption_change;
double T_station_platform;
double D_station_platform = 500;