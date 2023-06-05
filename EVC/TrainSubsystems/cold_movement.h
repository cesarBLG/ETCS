/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;
enum ColdMovement
{
    NoColdMovement,
    ColdMovement,
    ColdMovementUnknown
};
extern int cold_movement_status;
void initialize_cold_movement();
void save_cold_data(std::string field, json &value);
json load_cold_data(std::string field);