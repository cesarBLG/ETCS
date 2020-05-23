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
#include "fixed_values.h"
#define TO_MPS(kph) kph/3.6
const double dV_ebi_min = TO_MPS(7.5);
const double dV_ebi_max = TO_MPS(15);
const double V_ebi_min = TO_MPS(110);
const double V_ebi_max = TO_MPS(210);

const double dV_sbi_min = TO_MPS(5.5);
const double dV_sbi_max = TO_MPS(10);
const double V_sbi_min = TO_MPS(110);
const double V_sbi_max = TO_MPS(210);

const double dV_warning_min = TO_MPS(4);
const double dV_warning_max = TO_MPS(5);
const double V_warning_min = TO_MPS(110);
const double V_warning_max = TO_MPS(140);

const double T_dispTTI = 14;
const double T_warning = 2;
const double T_driver = 4;
const double T_horn = 4;
const double T_delete_condition = 5;

const double M_rotating_max = 0.15;
const double M_rotating_min = 0.02;

const double T_CYCRQSTD = 60;
const double T_ACK = 5;

const double D_Metal = 300;

const double D_keep_information = 300;