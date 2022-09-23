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
#include "level_transition.h"
#include "../Packets/messages.h"
#include "../Packets/information.h"
#include "../DMI/text_message.h"
#include "../TrainSubsystems/brake.h"
#include "../TrainSubsystems/cold_movement.h"
#include "../STM/stm.h"
optional<level_transition_information> ongoing_transition;
optional<level_transition_information> sh_transition;
std::vector<level_information> priority_levels;
bool priority_levels_valid = false;
optional<distance> transition_border;
Level level = Level::Unknown;
int nid_ntc = -1;
bool level_valid = false;
std::list<std::list<std::shared_ptr<etcs_information>>> transition_buffer;
bool level_acknowledgeable = false;
bool level_acknowledged = false;
Level level_to_ack;
int ntc_to_ack;
bool level_timer_started = false;
int64_t level_timer;
std::map<int, std::string> ntc_names;
#include <fstream>
void load_level()
{
    //std::ifstream file("level.dat");
    if (cold_movement_status == NoColdMovement) {
        level_valid = true;
        priority_levels_valid = true;
    } else {
        level_valid = false;
        priority_levels.clear();
    }
}
void save_level()
{
    //std::ofstream file("level.dat");
}
void driver_set_level(level_information li)
{
    stm_level_change(li, true);
    level_valid = li.level != Level::Unknown;
    position_report_reasons[6] = (li.level != Level::N2 && li.level != Level::N3 && (level == Level::N2 || level == Level::N3)) ? 2 : 1;
    level = li.level;
    if (level == Level::NTC) {
        nid_ntc = li.nid_ntc;
    } else {
        nid_ntc = -1;
    }
    save_level();
}
void perform_transition()
{
    if(!ongoing_transition)
        return;
    level_transition_information lti = *ongoing_transition;
    stm_level_change({lti.leveldata.level, lti.leveldata.nid_ntc}, false);
    if (level == Level::N2 || level == Level::N3)
        transition_border = lti.start;
    else
        transition_border = {};
    priority_levels = lti.priority_table;
    priority_levels_valid = true;
    ongoing_transition = {};
    STM_max_speed = {};
    STM_system_speed = {};
    if (level_to_ack == Level::NTC || level == Level::NTC || level_to_ack == Level::N0)
        level_acknowledgeable = !level_acknowledged;
    Level prevlevel = level;
    level = lti.leveldata.level;
    if (level == Level::NTC)
        nid_ntc = lti.leveldata.nid_ntc;
    else
        nid_ntc = -1;
    for (auto it=transition_buffer.begin(); it!=transition_buffer.end(); ++it) {
        for (auto it2 = it->begin(); it2!=it->end(); ++it2) {
            try_handle_information(*it2, *it);
        }
    }
    transition_buffer.clear();
    if (level_acknowledgeable && !level_acknowledged) {
        level_timer_started = true;
        level_timer = get_milliseconds();
    }
    position_report_reasons[6] = true;
    save_level();
}
void update_level_status()
{
    if (level_timer_started && level_timer + T_ACK*1000 < get_milliseconds()) {
        level_timer_started = false;
        brake_conditions.push_back({-1, nullptr, [](brake_command_information &i) {
            if (level_acknowledged || level_to_ack != level || (level == Level::NTC && nid_ntc != ntc_to_ack) || !level_acknowledgeable)
                return true;
            return false;
        }});
    }
    if (transition_border && d_minsafefront(*transition_border)-L_TRAIN > *transition_border && (level != Level::N2 && level != Level::N3)) {
        position_report_reasons[5] = true;
        transition_border = {};
    }
    if (sh_transition && mode != Mode::SH && mode != Mode::PS) {
        ongoing_transition = sh_transition;
        sh_transition = {};
        level_to_ack = ongoing_transition->leveldata.level;
        ntc_to_ack = ongoing_transition->leveldata.nid_ntc;
        perform_transition();
    }
    if (!ongoing_transition) return;
    if (ongoing_transition->start<=d_estfront)
        perform_transition();
    else if (mode != Mode::SB && 
    (level_to_ack == Level::NTC || level == Level::NTC || level_to_ack == Level::N0) && 
    ongoing_transition->leveldata.startack < d_maxsafefront(ongoing_transition->leveldata.startack) && !level_acknowledged) {
        level_acknowledgeable = true;
    }
}
void level_transition_received(level_transition_information info)
{
    stm_level_transition_received(info);
    if (!ongoing_transition || ongoing_transition->leveldata.level != info.leveldata.level || (ongoing_transition->leveldata.level == Level::NTC && ongoing_transition->leveldata.nid_ntc == info.leveldata.nid_ntc)) {
        level_acknowledged = false;
        STM_max_speed = {};
        STM_system_speed = {};
    }
    level_acknowledgeable = false;
    level_timer_started = false;
    transition_buffer.clear();
    transition_buffer.push_back({});
    if (info.leveldata.level == level && (level != Level::NTC || info.leveldata.nid_ntc == nid_ntc)) {
        ongoing_transition = {};
        STM_max_speed = {};
        STM_system_speed = {};
        priority_levels = info.priority_table;
        priority_levels_valid = true;
        save_level();
        return;
    }
    if (mode == Mode::SH || mode == Mode::PS) {
        sh_transition = info;
        priority_levels = info.priority_table;
        ongoing_transition = {};
        STM_max_speed = {};
        STM_system_speed = {};
        priority_levels_valid = true;
        save_level();
        return;
    }
    ongoing_transition = info;
    level_to_ack = ongoing_transition->leveldata.level;
    ntc_to_ack = ongoing_transition->leveldata.nid_ntc;
    if (ongoing_transition->immediate)
        perform_transition();
}
level_transition_information::level_transition_information(LevelTransitionOrder o, distance ref)
{
    if (o.D_LEVELTR == D_LEVELTR_t::Now) {
        immediate = true;
        start = ref;
    } else {
        immediate = false;
        start = ref+o.D_LEVELTR.get_value(o.Q_SCALE);
    }
    std::vector<target_level_information> priorities;
    priorities.push_back({start-o.element.L_ACKLEVELTR.get_value(o.Q_SCALE), o.element.M_LEVELTR.get_level(),(int)o.element.NID_NTC});
    for (int i=0; i<o.elements.size(); i++) {
        priorities.push_back({start-o.elements[i].L_ACKLEVELTR.get_value(o.Q_SCALE), o.elements[i].M_LEVELTR.get_level(),  (int)o.elements[i].NID_NTC});
    }
    for (int i=0; i<priorities.size(); i++) {
        priority_table.push_back({priorities[i].level, priorities[i].nid_ntc});
    }
    set_leveldata(priorities);
}
level_transition_information::level_transition_information(ConditionalLevelTransitionOrder o, distance ref)
{
    immediate = true;
    start = ref;
    std::vector<target_level_information> priorities;
    priorities.push_back({start, o.element.M_LEVELTR.get_level(), (int)o.element.NID_NTC});
    for (int i=0; i<o.elements.size(); i++) {
        priorities.push_back({start, o.elements[i].M_LEVELTR.get_level(), (int)o.elements[i].NID_NTC});
    }
    for (int i=0; i<priorities.size(); i++) {
        if (priorities[i].level == level && (level != Level::NTC || priorities[i].nid_ntc == nid_ntc)) {
            leveldata = priorities[i];
            return;
        }
    }
    set_leveldata(priorities);
}
void level_transition_information::set_leveldata(std::vector<target_level_information> &priorities)
{
    for (auto &p : priorities) {
        if (p.level == Level::NTC) {
            assign_stm(p.nid_ntc, false);
            auto *stm = get_stm(p.nid_ntc);
            if (stm != nullptr && stm->available()) {
                leveldata = p;
                return;
            }
        }
        if (p.level == Level::N2) {
            for (auto &t : mobile_terminals) {
                leveldata = p;
                return;
            }
        }
        if (p.level == Level::N0 || p.level == Level::N1) {
            leveldata = p;
            return;
        }
    }
    leveldata = priorities.back();
}