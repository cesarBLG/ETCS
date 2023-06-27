/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "cold_movement.h"
#include "../Packets/vbc.h"
#include "../Supervision/national_values.h"
#include "../language/language.h"
#include "../Euroradio/session.h"
#include "../Position/linking.h"
#include "platform_runtime.h"
int cold_movement_status;
void initialize_cold_movement()
{
    cold_movement_status = ColdMovementUnknown;
    load_language();
    setup_national_values();
    load_vbcs();
    load_contact_info();
    load_train_position();
}
void save_cold_data(std::string field, json &value)
{
    auto contents = platform->read_file("cold_data.json");
    if (!contents)
        return;
    json j = json::parse(*contents);
    j[field] = value;
    platform->write_file("cold_data.json", j.dump());
}
json load_cold_data(std::string field)
{
    auto contents = platform->read_file("cold_data.json");
    if (!contents)
        return json();
    json j = json::parse(*contents);
    if (!j.contains(field)) j[field] = json();
    return j[field];
}