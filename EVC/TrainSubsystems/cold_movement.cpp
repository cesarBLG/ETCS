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
#include <fstream>
int cold_movement_status;
static std::string filename;
void initialize_cold_movement()
{
    filename = "cold_data.json";
    cold_movement_status = ColdMovementUnknown;
    load_language();
    setup_national_values();
    load_vbcs();
    load_contact_info();
    load_train_position();
}
void save_cold_data(std::string field, json &value)
{
    std::ifstream in(filename);
    json j;
    if (in.good()) in >> j;
    j[field] = value;
    std::ofstream out(filename); 
    out << j;
}
json load_cold_data(std::string field)
{
    std::ifstream in(filename);
    if (!in.good()) return json();
    json j;
    in >> j;
    if (!j.contains(field)) j[field] = json();
    return j[field];
}