/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "acks.h"
#include "windows.h"
#include "../Time/clock.h"
#include "text_message.h"
#include "../Procedures/mode_transition.h"
#include "../Procedures/level_transition.h"
#include "../STM/stm.h"
bool ack_required;
bool ack_allowed;
int64_t last_ack_time;
void update_acks()
{
    ack_required = mode_acknowledgeable || level_acknowledgeable || brake_acknowledgeable;
    for (auto &msg : messages) {
        if (msg.ack && !msg.acknowledged)
            ack_required = true;
    }
    if (ack_required && active_dialog != dialog_sequence::StartUp) {
        if (active_dialog != dialog_sequence::None) {
            std::string active = active_window_dmi["active"];
            if (active != "menu_main" && active != "menu_override" && active != "menu_spec" && 
            active != "menu_settings" && active != "menu_radio" && active != "menu_ntc") {
                close_window();
                last_ack_time = get_milliseconds();
            }
        }
        if (get_milliseconds() > last_ack_time + 1000)
            ack_allowed = true;
    } else {
        ack_allowed = false;
    }
}