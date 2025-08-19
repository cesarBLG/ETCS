/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <limits>
#include <map>
#include <optional>
#include "../Packets/3.h"
#include "../Position/distance.h"
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

extern std::set<int> NV_NID_Cs;

struct StoredNationalValueSet
{
    distance first_applicable;
    NationalValues nv;
};
extern std::optional<StoredNationalValueSet> not_yet_applicable_nv;

void setup_national_values();
void national_values_received(NationalValues nv, std::optional<distance> reference);
void update_national_values();
void reset_national_values();