/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "config.h"
#include "../DMI/dmi.h"
#include "../Procedures/level_transition.h"
#include "../STM/stm.h"
#include <nlohmann/json.hpp>
#include "platform_runtime.h"
using json = nlohmann::json;
extern std::string traindata_file;
extern int data_entry_type;
void load_config(std::string serie)
{
    traindata_file = "traindata.json";
    data_entry_type = 0;
    json j;
    auto contents = platform->read_file("config.json");
    if (contents)
        j = json::parse(*contents);
    if (j.contains(serie)) {
        json &cfg = j[serie];
        if (cfg.contains("TrainData")) {
            traindata_file = cfg["TrainData"];
            data_entry_type = 1;
        }
        if (cfg.contains("UnsupportedLevels")) {
            unsupported_levels = cfg["UnsupportedLevels"].get<std::set<int>>();
        }
        if (cfg.contains("AvailableNTC")) {
            std::set<int> ntcs = cfg["AvailableNTC"].get<std::set<int>>();
            for (int ntc : ntcs)
            {
                ntc_available_no_stm.insert(ntc);
            }
        }
        if (cfg.contains("InstalledSTM")) {
            std::set<int> stms = cfg["InstalledSTM"].get<std::set<int>>();
            for (auto it = installed_stms.begin(); it != installed_stms.end(); ) {
                if (stms.find(it->second->nid_stm) == stms.end()) {
                    delete it->second;
                    it = installed_stms.erase(it);
                } else {
                    ++it;
                }
            }
            for (int nid_stm : stms) {
                if (installed_stms.find(nid_stm) == installed_stms.end()) {
                    installed_stms[nid_stm] = new stm_object();
                    installed_stms[nid_stm]->nid_stm = nid_stm;
                }
            }
        }
        if (cfg.contains("STMPriorityTable")) {
            ntc_to_stm_lookup_table.clear();
            for (json &j : cfg["STMPriorityTable"])
            {
                ntc_to_stm_lookup_table[j["nid_ntc"].get<int>()] = j["nid_stms"].get<std::vector<int>>();
            }
        }
    }
    set_persistent_command("setSerie", serie);
}