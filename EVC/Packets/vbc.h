/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <list>
#include "../Time/clock.h"
struct virtual_balise_cover
{
    int NID_C;
    int NID_VBCMK;
    int64_t set_time;
    int64_t validity;
};
extern std::list<virtual_balise_cover> vbcs;
void set_vbc(virtual_balise_cover vbc);
void remove_vbc(virtual_balise_cover vbc);
bool vbc_ignored(int nid_c, int nid_vbcmk);
void load_vbcs();
void update_vbc();