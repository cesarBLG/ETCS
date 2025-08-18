/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "vbc.h"
#include "../TrainSubsystems/cold_movement.h"
std::list<virtual_balise_cover> vbcs;
void from_json(const json&j, virtual_balise_cover &vbc)
{
    vbc.NID_C = j["NID_C"].get<int>();
    vbc.NID_VBCMK = j["NID_VBCMK"].get<int>();
    vbc.set_time = j.value<int64_t>("SetTime", 0);
    vbc.validity = j["Validity"].get<int64_t>();
}
void to_json(json&j, const virtual_balise_cover &vbc)
{
    j["NID_C"] = vbc.NID_C;
    j["NID_VBCMK"] = vbc.NID_VBCMK;
    j["SetTime"] = vbc.set_time;
    j["Validity"] = vbc.validity;
}
void write_vbcs()
{
    std::vector<json> vbcj;
    for (auto &vbc : vbcs) {
        vbcj.push_back(vbc);
    }
    json j = vbcj;
    save_cold_data("VBC", j);
}
void load_vbcs()
{
    json vbcj = load_cold_data("VBC");
    if (vbcj.is_null()) return;
    for (auto &vbc : vbcj) {
        vbcs.push_back(vbc);
    }
}
void set_vbc(virtual_balise_cover vbc)
{
    for (auto it = vbcs.begin(); it != vbcs.end(); ) {
        if (vbc.NID_C == it->NID_C && vbc.NID_VBCMK == it->NID_VBCMK)
            it = vbcs.erase(it);
        else
            ++it;
    }
    vbcs.push_back(vbc);
    write_vbcs();
}
void remove_vbc(virtual_balise_cover vbc)
{
    for (auto it = vbcs.begin(); it != vbcs.end(); ) {
        if (vbc.NID_C == it->NID_C && vbc.NID_VBCMK == it->NID_VBCMK)
            it = vbcs.erase(it);
        else
            ++it;
    }
    write_vbcs();
}
void update_vbc()
{
    for (auto it = vbcs.begin(); it != vbcs.end(); ) {
        if (it->validity + it->set_time > get_milliseconds())
            it = vbcs.erase(it);
        else
            ++it;
    }
}
bool vbc_ignored(int nid_c, int nid_vbcmk)
{
    for (auto &vbc : vbcs) {
        if (vbc.NID_C == nid_c && vbc.NID_VBCMK == nid_vbcmk)
            return true;
    }
    return false;
}