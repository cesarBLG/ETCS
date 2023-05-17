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
#include "../Time/clock.h"
struct virtual_balise_cover
{
    int NID_C;
    int NID_VBCMK;
    int64_t validity;
    bool operator<(const virtual_balise_cover &o) const
    {
        return NID_VBCMK < o.NID_VBCMK;
    }
};
extern std::set<virtual_balise_cover> vbcs;
void set_vbc(virtual_balise_cover vbc);
void remove_vbc(virtual_balise_cover vbc);
bool vbc_ignored(int nid_c, int nid_vbcmk);
void load_vbcs();