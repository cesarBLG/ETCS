/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "config.h"
#include "platform_runtime.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
extern int maxSpeed;
extern int etcsDialMaxSpeed;
extern bool softkeys;
extern bool serieSelected;
extern bool playSoundOnRadioStatusChange;
extern bool displayTTPavailable;
extern std::string stm_layout_file;
void startWindows();
void load_config(std::string serie)
{
    serieSelected = false;
    etcsDialMaxSpeed = 400;
    stm_layout_file = "stm_windows.json";
    auto contents = platform->read_file("config.json");
    if (contents) {
        json j = json::parse(*contents);
        if (j.contains(serie)) {
            json &cfg = j[serie];
            etcsDialMaxSpeed = cfg.value("SpeedDial", 400);
            if (cfg.contains("STMLayout")) {
                stm_layout_file = cfg["STMLayout"];
            }
            playSoundOnRadioStatusChange = cfg.value("PlaySoundOnRadioStatusChange", false);
            displayTTPavailable = cfg.value("DisplayTimeToPermitted", false);
            bool sk = cfg.contains("SoftKeys") && cfg["SoftKeys"];
            if (softkeys != sk) {
                softkeys = sk;
                startWindows();
            }
            serieSelected = true;
        }
    } else {
        platform->debug_print("failed to load config.json");
    }
    maxSpeed = etcsDialMaxSpeed;
}
