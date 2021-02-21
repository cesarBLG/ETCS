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
#include <limits>
#include <map>
#include "../Packets/3.h"
extern bool Q_NVDRIVER_ADHES;

extern double V_NVSHUNT;
extern double V_NVSTFF;
extern double V_NVONSIGHT;
extern double V_NVLIMSUPERV;
extern double V_NVUNFIT;
extern double V_NVREL;

extern double D_NVROLL;

extern bool Q_NVSBTSMPERM;
extern bool Q_NVEMRRLS;
extern bool Q_NVGUIPERM;
extern bool Q_NVSBFBPERM;
extern bool Q_NVINHSMICPERM;

extern double V_NVALLOWOVTRP;
extern double V_NVSUPOVTRP;

extern double D_NVOVTRP;

extern double T_NVOVTRP;

extern int M_NVDERUN;
extern int M_NVCONTACT;

extern double T_NVCONTACT;

extern double D_NVPOTRP;

extern double D_NVSTFF;

extern double Q_NVLOCACC;

extern int M_NVAVADH;

extern double M_NVEBCL;

struct kvint_pass_step
{
    double a;
    double b;
    double A_NVP12;
    double A_NVP23;
};
extern std::map<double,double> NV_KRINT;
extern std::map<double,double> NV_KVINT_freight;
extern std::map<double,kvint_pass_step> NV_KVINT_pass;
extern double M_NVKTINT;

extern double A_NVMAXREDADH1;
extern double A_NVMAXREDADH2;
extern double A_NVMAXREDADH3;
 
void setup_national_values();
void national_values_received(NationalValues nv, distance reference);
void update_national_values();