/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "distance.h"
#include "../Packets/5.h"
#include <list>
#include "../optional.h"
struct link_data
{
    bg_id nid_bg;
    distance dist;
    double locacc;
    bool reverse_dir;
    int reaction;
    distance max()
    {
        return dist+locacc;
    }
    distance min()
    {
        return dist-locacc;
    }
};
struct lrbg_info
{
    bg_id nid_lrbg;
    int dir;
    distance position;
    double locacc;
};
extern std::list<link_data> linking;
extern std::list<lrbg_info> lrbgs;
extern bool position_valid;
distance update_location_reference(bg_id nid_bg, int dir, distance group_pos, bool linked, optional<link_data> link);
void update_linking(distance start, Linking link, bool infill, bg_id this_bg);
void delete_linking();
void delete_linking(distance from);
void load_train_position();
void save_train_position();