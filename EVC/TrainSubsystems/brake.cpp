/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <functional>
#include <list>
#include <algorithm>
#include "train_interface.h"
#include "../Time/clock.h"
#include "brake.h"
#include "../Procedures/stored_information.h"
#include "../Euroradio/session.h"
#include "../STM/stm.h"
#include "../language/language.h"
extern bool SB;
extern bool EB;
bool brake_acknowledgeable;
bool brake_acknowledged;
std::list<brake_command_information> brake_conditions;
void trigger_brake_reason(int reason)
{
    for (auto cond : brake_conditions) {
        if (cond.reason == reason) return;
    }
    if (reason == 0) {
        text_message msg(get_text("Balise read error"), true, false, 2, [](text_message &msg){return false;});
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
        extern bool traindata_applied;
        text_message msg(get_text("Runaway movement"), true, false, 2, [](text_message &msg){return !standstill_applied && !rollaway_applied && !rmp_applied && !pt_applied && !traindata_applied;});
        text_message *m = &add_message(msg);
        brake_conditions.push_back({reason, m, [](brake_command_information &i) {
            if (!standstill_applied && !rollaway_applied && !rmp_applied && !pt_applied && !traindata_applied) {
                brake_acknowledgeable = false;
                return true;
            }
            if (V_est == 0 && !brake_acknowledgeable) {
                brake_acknowledgeable = true;
                brake_acknowledged = false;
            }
            return false;
        }});
    } else if (reason == 2) {
        text_message msg(get_text("Communication error"), true, false, 2, [](text_message &msg){return false;});
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
    } else if (reason == 3) {
        text_message msg(get_text("Train data changed"), true, false, 2, [](text_message &msg){return false;});
        text_message *m = &add_message(msg);
        brake_conditions.push_back({reason, m, [](brake_command_information &i) {
            if (V_est == 0) {
                int64_t time = get_milliseconds();
                i.msg->end_condition = [time](text_message &m) {
                    return time+30000<get_milliseconds();
                };
                train_shorten('j');
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
    if (mode == Mode::IS)
    {
        EB_command = SB_command = false;
        brake_conditions.clear();
        return;
    }
    for (auto it = brake_conditions.begin(); it!=brake_conditions.end();) {
        if (it->revoke(*it))
            it = brake_conditions.erase(it);
        else
            ++it;
    }
    SB_command = !brake_conditions.empty();
    SB_command |= SB;
    EB_command = EB;
    EB_command |= stm_control_EB;
    if (mode == Mode::SN) {
        for (auto kvp : installed_stms) {
            auto *stm = kvp.second;
            if (stm->active()) {
                SB_command |= stm->biu_function.SB;
                EB_command |= stm->biu_function.EB;
            }
        }
    }
    if ((prevEB || prevSB) && !EB_command && !SB_command) send_command("playSinfo","");
    prevEB = EB_command;
    prevSB = SB_command;
}