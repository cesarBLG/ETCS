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
#include <functional>
#include <list>
#include <algorithm>
#include "../Time/clock.h"
#include "brake.h"
#include "../Procedures/stored_information.h"
#include "../Euroradio/session.h"
extern bool SB;
extern bool EB;
bool SB_commanded;
bool EB_commanded;
bool brake_acknowledgeable;
bool brake_acknowledged;
std::list<brake_command_information> brake_conditions;
void trigger_brake_reason(int reason)
{
    for (auto cond : brake_conditions) {
        if (cond.reason == reason) return;
    }
    if (reason == 0) {
        text_message msg("Balise read error", true, false, 2, [](text_message &msg){return false;});
        text_message *m = &add_message(msg);
        brake_conditions.push_back({reason, m, [](brake_command_information &i) {
            if (V_est == 0) {
                int64_t time = get_milliseconds();
                i.msg->end_condition = [time](text_message &m) {
                    return time+30000<get_milliseconds();
                };
                train_shorten('g');
                send_command("playSinfo","");
                return true;
            }
            return false;
        }});
    } else if (reason == 1) {
        extern bool standstill_applied;
        extern bool rollaway_applied;
        extern bool rmp_applied;
        extern bool pt_applied;
        text_message msg("Runaway movement", true, false, 2, [](text_message &msg){return !standstill_applied && !rollaway_applied && !rmp_applied && !pt_applied;});
        text_message *m = &add_message(msg);
        brake_conditions.push_back({reason, m, [](brake_command_information &i) {
            if (!standstill_applied && !rollaway_applied && !rmp_applied && !pt_applied) {
                brake_acknowledgeable = false;
                i.msg->end_condition = [](text_message&m){return true;};
                return true;
            }
            if (V_est == 0 && !brake_acknowledgeable) {
                brake_acknowledgeable = true;
                brake_acknowledged = false;
            }
            return false;
        }});
    } else if (reason == 2) {
        text_message msg("Communication error", true, false, 2, [](text_message &msg){return false;});
        text_message *m = &add_message(msg);
        brake_conditions.push_back({reason, m, [](brake_command_information &i) {
            if (V_est == 0) {
                int64_t time = get_milliseconds();
                i.msg->end_condition = [time](text_message &m) {
                    return time+30000<get_milliseconds();
                };
                train_shorten('i');
                send_command("playSinfo","");
                return true;
            }
            if (!radio_reaction_applied) {
                i.msg->end_condition = [](text_message &m) {
                    return m.first_displayed+30000<get_milliseconds();
                };
                send_command("playSinfo","");
                return true;
            }
            return false;
        }});
    }
}
static bool prevEB;
static bool prevSB;
void handle_brake_command()
{
    for (auto it = brake_conditions.begin(); it!=brake_conditions.end();) {
        if (it->revoke(*it))
            it = brake_conditions.erase(it);
        else
            ++it;
    }
    SB_commanded = !brake_conditions.empty();
    SB_commanded |= SB;
    EB_commanded = EB;
    if ((prevEB || prevSB) && !EB_commanded && !SB_commanded) send_command("playSinfo","");
    prevEB = EB;
    prevSB = SB;
}