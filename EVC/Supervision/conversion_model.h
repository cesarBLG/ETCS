/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <map>
#include "acceleration.h"
#include <nlohmann/json.hpp>
#define REGENERATIVE_AVAILABLE 0
#define EDDY_AVAILABLE 1
#define EP_AVAILABLE 2
#define MAGNETIC_AVAILABLE 3
using json = nlohmann::json;
void set_brake_model(json &traindata);
void set_conversion_model();
acceleration get_A_gradient(std::map<distance, double> gradient, double default_gradient);
extern double T_brake_emergency_cm0;
extern double T_brake_emergency_cmt;
extern double T_brake_service_cm0;
extern double T_brake_service_cmt;
double get_T_brake_emergency(distance d);
double get_T_brake_service(distance d);
acceleration get_A_brake_emergency(bool use_active_combination=true);
acceleration get_A_brake_service(bool use_active_combination=true);
acceleration get_A_brake_normal_service(acceleration A_brake_service);
extern double Kt_int;
extern std::map<double, double> Kv_int;
extern std::map<double, double> Kr_int;
extern std::map<double,double> Kn[2];
extern bool conversion_model_used;
double Kdry_rst(double V, double EBCL, distance d);
double Kwet_rst(double V, distance d);
extern std::map<distance,std::pair<int,int>> active_combination;
extern bool slippery_rail_driver;