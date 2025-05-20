/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "../variables.h"
namespace V1
{
struct M_AXLELOAD_t : ETCS_variable
{
    uint32_t Above40t = 126;
    M_AXLELOAD_t() : ETCS_variable(7) {}
    bool is_valid(int m_version) override
    {
        return rawdata < 81 || rawdata == Above40t;
    }
};
struct M_MAMODE_t : ETCS_variable
{
    uint32_t OS=0;
    uint32_t SH=1;
    M_MAMODE_t() : ETCS_variable(2) 
    {
        invalid.insert(2);
        invalid.insert(3);
    }
};
struct M_POSITION_t : ETCS_variable
{
    uint32_t NoMoreCalculation=1048575;
    M_POSITION_t() : ETCS_variable(20) {}
};
struct M_TRACTION_t : ETCS_variable
{
    M_TRACTION_t() : ETCS_variable(8) {}
};
struct NC_DIFF_t : ETCS_variable
{
    uint32_t CD275=0;
    uint32_t CD80=1;
    uint32_t CD100=2;
    uint32_t CD130=3;
    uint32_t CD150=4;
    uint32_t CD165=5;
    uint32_t CD180=6;
    uint32_t CD225=7;
    uint32_t CD300=8;
    uint32_t FreightP=9;
    uint32_t FreightG=10;
    uint32_t Passenger=11;
    uint32_t CD245=12;
    uint32_t CD210=13;
    NC_DIFF_t() : ETCS_variable(4)
    {
        invalid.insert(14);
        invalid.insert(15);
    }
};
struct Q_TRACKDEL_t : ETCS_variable
{
    Q_TRACKDEL_t() : ETCS_variable(1) {}
};
}