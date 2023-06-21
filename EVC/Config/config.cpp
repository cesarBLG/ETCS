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
#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;
extern std::string traindata_file;
extern int data_entry_type;
void load_config(std::string serie)
{
#ifdef __ANDROID__
    extern std::string filesDir;
    std::ifstream file(filesDir+"/config.json");
#else
    std::ifstream file("config.json");
#endif
    traindata_file = "traindata.json";
    data_entry_type = 0;
    json j;
    file >> j;
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
    }
    set_persistent_command("setSerie", serie);
}