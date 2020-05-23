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
#include <set>
#include "../antenna.h"
#include <string>
enum brake_position_types
{
    PassengerP,
    FreightP,
    FreightG
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
extern std::string special_train_data;
bool train_data_valid();
void validate_train_data();
void invalidate_train_data();