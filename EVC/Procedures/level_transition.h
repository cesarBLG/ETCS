/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "../Position/distance.h"
#include "../Supervision/supervision.h"
#include "../Packets/41.h"
#include "../Packets/46.h"
#include "../Packets/messages.h"
#include "../Packets/etcs_information.h"
#include "../optional.h"
#include <vector>
#include <list>
struct level_information
{
    Level level;
    int nid_ntc;
};
extern std::vector<level_information> priority_levels;
struct target_level_information
{
    double acklength;
    Level level;
    int nid_ntc;
};
struct level_transition_information
{
    bool immediate;
    bool acknowledged = false;
    std::optional<distance> ref_loc;
    double dist;
    target_level_information leveldata;
    std::vector<level_information> priority_table;
    level_transition_information() = default;
    level_transition_information(LevelTransitionOrder o);
    level_transition_information(ConditionalLevelTransitionOrder o);
    void set_leveldata(std::vector<target_level_information> &priorities);
};
void update_level_status();
void level_transition_received(level_transition_information info);
void driver_set_level(level_information level);
void load_level();
extern optional<level_transition_information> ongoing_transition;
extern optional<level_transition_information> sh_transition;
extern optional<distance> max_ack_distance;
extern std::vector<level_information> priority_levels;
extern bool priority_levels_valid;
extern std::list<std::list<std::shared_ptr<etcs_information>>> transition_buffer;
extern bool level_acknowledgeable;
extern bool level_acknowledged;
extern Level level_to_ack;
extern int ntc_to_ack;
extern std::map<int, std::string> ntc_names;
extern std::set<int> ntc_available_no_stm;
extern std::set<int> unsupported_levels;