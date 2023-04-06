/*
 * European Train Control System
 * Copyright (C) 2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    distance startack;
    Level level;
    int nid_ntc;
};
struct level_transition_information
{
    bool immediate;
    bool acknowledged = false;
    distance start;
    target_level_information leveldata;
    std::vector<level_information> priority_table;
    level_transition_information(LevelTransitionOrder o, distance ref);
    level_transition_information(ConditionalLevelTransitionOrder o, distance ref);
    void set_leveldata(std::vector<target_level_information> &priorities);
};
void update_level_status();
void level_transition_received(level_transition_information info);
void driver_set_level(level_information level);
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