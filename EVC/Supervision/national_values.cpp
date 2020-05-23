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
#include "national_values.h"
#define TO_MPS(kph) kph/3.6
#include <limits>
bool Q_NVDRIVER_ADHES;

double V_NVSHUNT;
double V_NVSTFF;
double V_NVONSIGHT;
double V_NVLIMSUPERV;
double V_NVUNFIT;
double V_NVREL;

double D_NVROLL;

bool Q_NVSBTSMPERM;
bool Q_NVEMRRLS;
bool Q_NVGUIPERM;
bool Q_NVSBFBPERM;
bool Q_NVINHSMICPERM;

double V_NVALLOWOVTRP;
double V_NVSUPOVTRP;

double D_NVOVTRP;

double T_NVOVTRP;

int M_NVDERUN;
int M_NVCONTACT;

double T_NVCONTACT;

double D_NVPOTRP;

double D_NVSTFF;

double Q_NVLOCACC;

int M_NVAVADH;

double M_NVEBCL;

std::map<double,double> NV_KRINT;
std::map<double,double> NV_KVINT_freight;
std::map<double,kvint_pass_step> NV_KVINT_pass;
double M_NVKTINT;

double A_NVMAXREDADH1;
double A_NVMAXREDADH2;
double A_NVMAXREDADH3;
void nv_changed()
{
    /*set_conversion_correction_values();
    SR_dist;
    SR_speed;
    recalculate_MRSP();*/
}
void setup_national_values()
{
    Q_NVDRIVER_ADHES = false;
    V_NVSHUNT = TO_MPS(30);
    V_NVSTFF = TO_MPS(40);
    V_NVONSIGHT = TO_MPS(30);
    V_NVLIMSUPERV = TO_MPS(100);
    V_NVUNFIT = TO_MPS(100);
    V_NVREL = TO_MPS(40);

    D_NVROLL = 2;

    Q_NVSBTSMPERM = true;
    Q_NVEMRRLS = false;
    Q_NVGUIPERM = false;
    Q_NVSBFBPERM = false;
    Q_NVINHSMICPERM = false;

    V_NVALLOWOVTRP = 0;
    V_NVSUPOVTRP = TO_MPS(30);

    D_NVOVTRP = 200;

    T_NVOVTRP = 60;

    M_NVDERUN = 0;
    M_NVCONTACT = 0;

    T_NVCONTACT = std::numeric_limits<double>::infinity();

    D_NVPOTRP = 200;

    D_NVSTFF = std::numeric_limits<double>::infinity();

    Q_NVLOCACC = 12;

    M_NVAVADH = 0;

    M_NVEBCL = 0.999999999;

    NV_KRINT[0] = 0.9;
    NV_KVINT_freight[0] = 0.7;
    NV_KVINT_pass[0] = {0.7,0.7,0,0};
    M_NVKTINT=1.1;

    A_NVMAXREDADH1 = 1;
    A_NVMAXREDADH2 = 0.7;
    A_NVMAXREDADH3 = 0.7;
    nv_changed();
}