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
            if (active_dialog == dialog_sequence::NTCData) {
                for (auto &kvp : installed_stms) {
                    auto *stm = kvp.second;
                    if (stm->data_entry == stm_object::data_entry_state::Driver)
                        stm->data_entry = stm_object::data_entry_state::Active;
                }
            }
            active_dialog = dialog_sequence::None;
            last_ack_time = get_milliseconds();
        }
        if (get_milliseconds() > last_ack_time + 1000)
            ack_allowed = true;
    } else {
        ack_allowed = false;
    }
}