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
optional<level_transition_information> ongoing_transition;
std::vector<level_information> priority_levels;
optional<distance> transition_border;
Level level = Level::Unknown;
int nid_ntc;
bool level_valid = false;
std::list<std::list<std::shared_ptr<etcs_information>>> transition_buffer;
bool level_acknowledgeable = false;
bool level_acknowledged = false;
Level level_to_ack;
bool level_timer_started = false;
int64_t level_timer;
void perform_transition()
{
    if (!ongoing_transition) return;
    level_transition_information lti = *ongoing_transition;
    if (level == Level::N2 || level == Level::N3)
        transition_border = lti.start;
    else
        transition_border = {};
    priority_levels = lti.priority_table;
    ongoing_transition = {};
    if (level_to_ack == Level::NTC || level == Level::NTC || level_to_ack == Level::N0)
        level_acknowledgeable = !level_acknowledged;
    Level prevlevel = level;
    level = lti.leveldata.level;
    nid_ntc = lti.leveldata.nid_ntc;
    for (auto it=transition_buffer.begin(); it!=transition_buffer.end(); ++it) {
        for (auto it2 = it->begin(); it2!=it->end(); ++it2) {
            try_handle_information(*it2, *it);
        }
    }
    /*if ((prevlevel == Level::N2 || prevlevel == Level::N3) && level != Level::N2 && level != Level::N3 && supervising_rbc)
        transition_border = lti.start;*/
    transition_buffer.clear();
    if (level_acknowledgeable && !level_acknowledged) {
        level_timer_started = true;
        level_timer = get_milliseconds();
    }
    position_report_reasons[6] = true;
}
void update_level_status()
{
    if (level_timer_started && level_timer + T_ACK*1000 < get_milliseconds()) {
        level_timer_started = false;
        brake_conditions.push_back({-1, nullptr, [](brake_command_information &i) {
            if (level_acknowledged || level_to_ack != level || !level_acknowledgeable)
                return true;
            return false;
        }});
    }
    if (transition_border && d_minsafefront(*transition_border)-L_TRAIN > *transition_border && (level != Level::N2 && level != Level::N3)) {
        position_report_reasons[5] = true;
        transition_border = {};
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
    if (!ongoing_transition || ongoing_transition->leveldata.level != info.leveldata.level)
        level_acknowledged = false;
    level_acknowledgeable = false;
    level_timer_started = false;
    transition_buffer.clear();
    transition_buffer.push_back({});
    if (info.leveldata.level == level && (level != Level::NTC || info.leveldata.nid_ntc == nid_ntc)) {
        ongoing_transition = {};
        priority_levels = info.priority_table;
        return;
    }
    ongoing_transition = info;
    level_to_ack = ongoing_transition->leveldata.level;
    if (ongoing_transition->immediate)
        perform_transition();
}