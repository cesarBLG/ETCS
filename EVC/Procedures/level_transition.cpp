/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "level_transition.h"
#include "../Packets/messages.h"
#include "../Packets/information.h"
#include "../DMI/text_message.h"
#include "../TrainSubsystems/brake.h"
#include "../TrainSubsystems/cold_movement.h"
#include "../STM/stm.h"
#include "../DMI/track_ahead_free.h"
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
std::set<int> ntc_available_no_stm;
std::set<int> unsupported_levels;
void from_json(const json &j, level_information &l)
{
    l.level = (Level)j["Level"].get<int>();
    if (l.level == Level::NTC)
        l.nid_ntc = j["NID_NTC"];
}
void to_json(json &j, const level_information &l)
{
    j["Level"] = (int)l.level;
    if (l.level == Level::NTC)
        j["NID_NTC"] = l.nid_ntc;
}
void load_level()
{
    json j = load_cold_data("Level");
    if (!j.is_null()) {
        level = (Level)j["Level"].get<int>();
        if (level == Level::NTC)
            nid_ntc = j["NID_NTC"];
        level_valid = cold_movement_status == NoColdMovement;
    } else {
        level_valid = false;
        level = Level::Unknown;
    }
    j = load_cold_data("TracksideLevels");
    if (!j.is_null() && cold_movement_status == NoColdMovement) {
        priority_levels = j;
        priority_levels_valid = true;
    } else {
        priority_levels_valid = false;
        priority_levels.clear();
        j = nullptr;
        save_cold_data("TracksideLevels", j);
    }
}
void save_level()
{
    json j;
    if (level_valid)
        j = level_information({level, nid_ntc});
    else
        j = nullptr;
    save_cold_data("Level", j);
    if (priority_levels_valid)
        j = priority_levels;
    else
        j = nullptr;
    save_cold_data("TracksideLevels", j);
}
void driver_set_level(level_information li)
{
    if (ongoing_transition && ongoing_transition->leveldata.level == li.level && (li.level != Level::NTC || ongoing_transition->leveldata.nid_ntc == li.nid_ntc)) {
        ongoing_transition = {};
        level_acknowledgeable = false;
    }
    stm_level_change(li, true);
    level_valid = li.level != Level::Unknown;
    position_report_reasons[6] = (li.level != Level::N2 && li.level != Level::N3 && (level == Level::N2 || level == Level::N3)) ? 2 : 1;
    level = li.level;
    if (level == Level::NTC) {
        nid_ntc = li.nid_ntc;
    } else {
        nid_ntc = -1;
    }
    if (level == Level::N0 || level == Level::NTC || level == Level::N1) {
        inhibit_revocable_tsr = false;
    }
    if (level == Level::N1) {
        pos_report_params = {};
        ma_params = {30000, (int64_t)(T_CYCRQSTD*1000), 30000};
        taf_request = {};
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
        transition_border = lti.ref_loc ? *lti.ref_loc : distance::from_odometer(d_estfront);
    else
        transition_border = {};
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
    for (auto &msg : transition_buffer) {
        handle_information_set(msg, true);
    }
    transition_buffer.clear();
    if (level_acknowledgeable && !level_acknowledged) {
        level_timer_started = true;
        level_timer = get_milliseconds();
    }
    position_report_reasons[6] = true;
    if (level == Level::N0 || level == Level::NTC || level == Level::N1) {
        inhibit_revocable_tsr = false;
    }
    if (level == Level::N1) {
        pos_report_params = {};
        ma_params = {30000, (int64_t)(T_CYCRQSTD*1000), 30000};
        taf_request = {};
    }
    if (prevlevel == Level::N2 || prevlevel == Level::N3) {
        bool supported = false;
        for (auto &lev : lti.priority_table) {
            if (lev.level == Level::N2 || lev.level == Level::N3) {
                supported = true;
                break;
            }
        }
        if (!supported) {
            for (auto *session : active_sessions) {
                if (session->status == session_status::Establishing)
                    session->finalize();
            }
        }
    }
    save_level();
}
void update_level_status()
{
    if (mode == Mode::IS && level != Level::Unknown) {
        driver_set_level({ Level::Unknown, -1 });
        return;
    }
    if (level_timer_started && level_timer + T_ACK*1000 < get_milliseconds()) {
        level_timer_started = false;
        brake_conditions.push_back({-1, nullptr, [](brake_command_information &i) {
            if (level_acknowledged || level_to_ack != level || (level == Level::NTC && nid_ntc != ntc_to_ack) || !level_acknowledgeable)
                return true;
            return false;
        }});
    }
    if (transition_border && d_minsafefront(*transition_border)-L_TRAIN > transition_border->min && (level != Level::N2 && level != Level::N3)) {
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
    if (ongoing_transition->immediate || (ongoing_transition->ref_loc->est + ongoing_transition->dist <= d_estfront))
        perform_transition();
    else if (mode != Mode::SB && 
    (level_to_ack == Level::NTC || level == Level::NTC || level_to_ack == Level::N0) && 
    (!ongoing_transition->ref_loc || ongoing_transition->ref_loc->max + ongoing_transition->dist - ongoing_transition->leveldata.acklength < d_maxsafefront(*ongoing_transition->ref_loc)) && !level_acknowledged) {
        level_acknowledgeable = true;
    }
}
void level_transition_received(level_transition_information info)
{
    stm_level_transition_received(info);
    if (!ongoing_transition || ongoing_transition->leveldata.level != info.leveldata.level || (ongoing_transition->leveldata.level == Level::NTC && ongoing_transition->leveldata.nid_ntc != info.leveldata.nid_ntc)) {
        level_acknowledged = false;
        STM_max_speed = {};
        STM_system_speed = {};
        transition_buffer.clear();
        transition_buffer.push_back({});
    }
    level_acknowledgeable = false;
    level_timer_started = false;

    priority_levels = info.priority_table;
    priority_levels_valid = true;
    if (info.leveldata.level == level && (level != Level::NTC || info.leveldata.nid_ntc == nid_ntc)) {
        ongoing_transition = {};
        STM_max_speed = {};
        STM_system_speed = {};
        save_level();
        return;
    }
    if (mode == Mode::SH || mode == Mode::PS) {
        sh_transition = info;
        ongoing_transition = {};
        STM_max_speed = {};
        STM_system_speed = {};
        save_level();
        return;
    }
    ongoing_transition = info;
    level_to_ack = ongoing_transition->leveldata.level;
    ntc_to_ack = ongoing_transition->leveldata.nid_ntc;
}
level_transition_information::level_transition_information(LevelTransitionOrder o)
{
    if (o.D_LEVELTR == o.D_LEVELTR.Now) {
        immediate = true;
    } else {
        immediate = false;
        dist = o.D_LEVELTR.get_value(o.Q_SCALE);
    }
    std::vector<target_level_information> priorities;
    priorities.push_back({o.element.L_ACKLEVELTR.get_value(o.Q_SCALE), o.element.M_LEVELTR.get_level(),(int)o.element.NID_NTC});
    for (int i=0; i<o.elements.size(); i++) {
        priorities.push_back({o.elements[i].L_ACKLEVELTR.get_value(o.Q_SCALE), o.elements[i].M_LEVELTR.get_level(),  (int)o.elements[i].NID_NTC});
    }
    for (int i=0; i<priorities.size(); i++) {
        priority_table.push_back({priorities[i].level, priorities[i].nid_ntc});
    }
    set_leveldata(priorities);
}
level_transition_information::level_transition_information(ConditionalLevelTransitionOrder o)
{
    immediate = true;
    std::vector<target_level_information> priorities;
    priorities.push_back({0, o.element.M_LEVELTR.get_level(), (int)o.element.NID_NTC});
    for (int i=0; i<o.elements.size(); i++) {
        priorities.push_back({0, o.elements[i].M_LEVELTR.get_level(), (int)o.elements[i].NID_NTC});
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
            if ((stm != nullptr && stm->available()) || (stm == nullptr && ntc_available_no_stm.find(p.nid_ntc) != ntc_available_no_stm.end())) {
                leveldata = p;
                return;
            }
        }
        if (p.level == Level::N2) {
            if (unsupported_levels.find(2) == unsupported_levels.end()) {
                for (auto *t : mobile_terminals) {
                    if (t->powered) {
                        leveldata = p;
                        return;
                    }
                }
            }
        }
        if (p.level == Level::N0 || p.level == Level::N1) {
            leveldata = p;
            return;
        }
    }
    leveldata = priorities.back();
}