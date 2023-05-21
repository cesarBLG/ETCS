/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "config.h"
#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;
extern int maxSpeed;
extern int etcsDialMaxSpeed;
extern std::string stm_layout_file;
void load_config(std::string serie)
{
#ifdef __ANDROID__
    extern std::string filesDir;
    std::ifstream file(filesDir+"/config.json");
#else
    std::ifstream file("../EVC/config.json");
#endif
    etcsDialMaxSpeed = 400;
    stm_layout_file = "stm_windows.json";
    json j;
    file >> j;
    if (j.contains(serie)) {
        json &cfg = j[serie];
        if (cfg.contains("SpeedDial")) {
            etcsDialMaxSpeed = cfg["SpeedDial"];
        }
        if (cfg.contains("STMLayout")) {
            stm_layout_file = cfg["STMLayout"];
        }
    }
    maxSpeed = etcsDialMaxSpeed;
}