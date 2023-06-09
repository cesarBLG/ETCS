/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <set>
#include "../antenna.h"
#include <string>
enum brake_position_types
{
    FreightP,
    FreightG,
    PassengerP,
};
extern brake_position_types brake_position;
extern double A_ebmax;
extern double L_TRAIN;
extern double T_brake_emergency;
extern double T_brake_service; 
extern double T_traction_cutoff;
extern double M_rotating_nom;
extern double V_train;
extern bool Q_airtight;
extern int axle_number;
extern int brake_percentage;
extern int cant_deficiency;
extern std::set<int> other_train_categories;
extern std::string axle_load_category;
extern std::string loading_gauge;
extern std::string train_category;
extern std::string special_train_data;
extern bool train_data_valid;
extern std::string traindata_file;
void set_train_data(std::string spec);