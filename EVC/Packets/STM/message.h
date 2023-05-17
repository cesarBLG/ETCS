/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "../packets.h"
#include "stm_variables.h"
#include <memory>
struct stm_message : public ETCS_message
{
    NID_STM_t NID_STM;
    L_MESSAGE_stm_t L_MESSAGE;
    std::vector<std::shared_ptr<ETCS_packet>> packets;
    stm_message() = default;
    stm_message(bit_manipulator &b);
    void write_to(bit_manipulator &b) override
    {
        NID_STM.copy(b);
        L_MESSAGE.copy(b);
        for (auto &pack : packets) {
            pack->write_to(b);
        }
        L_MESSAGE.rawdata = b.bits.size();
        b.replace(&L_MESSAGE, 8);
        b.log_entries[1].second = std::to_string(L_MESSAGE.rawdata);
    }
};