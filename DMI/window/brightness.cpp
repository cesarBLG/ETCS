/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "brightness.h"
#include "../tcp/server.h"
#include <cstdlib>
#include "platform_runtime.h"
brightness_window::brightness_window() : input_window("Brightness", 1, false)
{
    int br_orig = platform->get_brightness();
    inputs[0] = new input_data("", false);
    inputs[0]->data = std::to_string(br_orig);
    inputs[0]->setAccepted(true);
    std::vector<Button*> keys;
    for (int i=0; i<12; i++) {
        keys.push_back(nullptr);
    }
    keys[0] = new TextButton(get_text("-"), 102, 50);
    keys[2] = new TextButton(get_text("+"), 102, 50);
    keys[0]->setPressedAction([this]
    {
        int br = stoi(inputs[0]->data);
        if (br >= 30) {
            br -= 10;
            platform->set_brightness(br);
            inputs[0]->setData(std::to_string(br));
        }
    });
    keys[2]->setPressedAction([this]
    {
        int br = stoi(inputs[0]->data);
        if (br <= 90) {
            br += 10;
            platform->set_brightness(br);
            inputs[0]->setData(std::to_string(br));
        }
    });
    keys[0]->upType = false;
    keys[2]->upType = false;
    inputs[0]->keys = keys;
    exit_button.setPressedAction([this, br_orig]
    {
        platform->set_brightness(br_orig);
        write_command("json",R"({"DriverSelection":"CloseWindow"})");
    });
    create();
}
void brightness_window::setLayout()
{
    input_window::setLayout();
}
void brightness_window::sendInformation()
{
    platform->set_brightness(stoi(inputs[0]->data_accepted));
    input_window::sendInformation();
}