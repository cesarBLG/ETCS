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
#include <list>
#include "locomotive_data.h"
#include <string>
enum brake_position_types
{
    FreightP,
    FreightG,
    PassengerP,
};
enum Electrifications
{
    NonElectrical,
    AC25KV,
    AC15KV,
    DC3KV,
    DC1500V,
    DC600_750V,
};
struct traction_type
{
    Electrifications electrification;
    int nid_ctraction;
};
enum struct axle_load_categories
{
    A,
    HS17,
    B1,
    B2,
    C2,
    C3,
    C4,
    D2,
    D3,
    D4,
    D4XL,
    E4,
    E5
};
enum struct loading_gauges
{
    G1,
    GA,
    GB,
    GC,
    OutGC,
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
extern axle_load_categories axle_load_category;
extern loading_gauges loading_gauge;
extern std::string train_category;
extern std::string special_train_data;
extern bool train_data_valid;
extern bool train_data_known;
extern std::string traindata_file;
extern std::list<traction_type> traction_systems;
void set_train_data(std::string spec);