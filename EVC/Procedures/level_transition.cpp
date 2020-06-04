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
Level level = Level::Unknown;
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
    ongoing_transition = {};
    if (level_to_ack == Level::NTC || level == Level::NTC || level_to_ack == Level::N0)
        level_acknowledgeable = !level_acknowledged;
    level = lti.leveldata.level;
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
}
void update_level_status()
{
    if (level_timer_started && level_timer + T_ACK*1000 < get_milliseconds()) {
        level_timer_started = false;
        brake_conditions.push_back({nullptr, [](brake_command_information &i) {
            if (level_acknowledged || level_to_ack != level || !level_acknowledgeable)
                return true;
            return false;
        }});
    }
    if (!ongoing_transition) return;
    if (ongoing_transition->start<=d_estfront)
        perform_transition();
    else if (mode != Mode::SB && 
    (level_to_ack == Level::NTC || level == Level::NTC || level_to_ack == Level::N0) && 
    ongoing_transition->leveldata.startack < d_maxsafefront(ongoing_transition->leveldata.startack.get_reference()) && !level_acknowledged) {
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
    if (info.leveldata.level == level) {
        ongoing_transition = {};
        return;
    }
    ongoing_transition = info;
    level_to_ack = ongoing_transition->leveldata.level;
    if (ongoing_transition->immediate)
        perform_transition();
}