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
    dist_base dist;
    double locacc;
    bool reverse_dir;
    int reaction;
    dist_base max()
    {
        return dist + locacc;
        //return dist.min+locacc;
    }
    dist_base min()
    {
        return dist - locacc;
        //return dist.max-locacc;
    }
};
struct lrbg_info
{
    bg_id nid_lrbg;
    int dir;
    dist_base position;
    int original_orientation;
    double locacc;
};
extern std::list<link_data> linking;
extern std::list<std::pair<lrbg_info, int>> orbgs;
extern std::optional<lrbg_info> solr;
extern bool position_valid;
void relocate();
void position_update_bg_passed(bg_id id, bool linked, dist_base pos, int dir);
void update_linking(Linking link, bool infill, bg_id ref_bg);
optional<distance> get_reference_location(bg_id bg, bool linked, bool check_passed);
void start_checking_linking();
void delete_linking();
void delete_linking(const distance &from);
void load_train_position();
void save_train_position();