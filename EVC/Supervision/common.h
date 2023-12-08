/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
enum MonitoringStatus
{
    CSM,
    TSM,
    RSM,
};
enum SupervisionStatus
{
    NoS,
    IndS,
    OvS,
    WaS,
    IntS
};
enum struct Level 
{
    N0,
    N1,
    N2,
#if BASELINE < 4
    N3,
#endif
    NTC,
    Unknown
};
enum struct Mode
{
    FS,
    LS,
    OS,
    SR,
    SH,
    UN,
    PS,
    SL,
    SB,
    TR,
    PT,
    SF,
    IS,
    NP,
    NL,
    SN,
    RV,
#if BASELINE == 4
    AD,
    SM,
#endif
};