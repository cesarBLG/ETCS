/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "config.h"
#include "../platform/platform.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
extern int maxSpeed;
extern int etcsDialMaxSpeed;
extern std::string stm_layout_file;
void load_config(std::string serie)
{
    etcsDialMaxSpeed = 400;
    stm_layout_file = "stm_windows.json";
    std::string contents = platform->read_file("config.json");
    if (!contents.empty()) {
        json j = json::parse(contents);
        if (j.contains(serie)) {
            json &cfg = j[serie];
            if (cfg.contains("SpeedDial")) {
                etcsDialMaxSpeed = cfg["SpeedDial"];
            }
            if (cfg.contains("STMLayout")) {
                stm_layout_file = cfg["STMLayout"];
            }
        }
    } else {
        platform->debug_print("failed to load config.json");
    }
    maxSpeed = etcsDialMaxSpeed;
}