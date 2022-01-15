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
#include <map>
#include "acceleration.h"
#include "../Parser/nlohmann/json.hpp"
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