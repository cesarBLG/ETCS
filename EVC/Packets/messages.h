/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
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
#include "variables.h"
#include "types.h"
#include "packets.h"
#include "radio.h"
#include "../Position/linking.h"
#include "../MA/movement_authority.h"
#include "../Supervision/speed_profile.h"
#include "../Euroradio/session.h"
#include "radio.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <deque>
struct eurobalise_telegram : public ETCS_message
{
    Q_UPDOWN_t Q_UPDOWN;
    M_VERSION_t M_VERSION;
    Q_MEDIA_t Q_MEDIA;
    N_PIG_t N_PIG;
    N_TOTAL_t N_TOTAL;
    M_DUP_t M_DUP;
    M_MCOUNT_t M_MCOUNT;
    NID_C_t NID_C;
    NID_BG_t NID_BG;
    Q_LINK_t Q_LINK;
    std::vector<std::shared_ptr<ETCS_packet>> packets;
    eurobalise_telegram(bit_manipulator &b)
    {
        extern double or_dist;
        Q_UPDOWN.copy(b);
        M_VERSION.copy(b);
        Q_MEDIA.copy(b);
        N_PIG.copy(b);
        N_TOTAL.copy(b);
        M_DUP.copy(b);
        M_MCOUNT.copy(b);
        NID_C.copy(b);
        NID_BG.copy(b);
        Q_LINK.copy(b);
        while (!b.error)
        {
            NID_PACKET_t NID_PACKET;
            b.peek(&NID_PACKET);
            if (NID_PACKET==255)
                break;
            packets.push_back(std::shared_ptr<ETCS_packet>(ETCS_packet::construct(b)));
        }
        readerror = b.error;
        valid = !b.sparefound;
    }
    void write_to(bit_manipulator &b) override
    {
        Q_UPDOWN.copy(b);
        M_VERSION.copy(b);
        Q_MEDIA.copy(b);
        N_PIG.copy(b);
        N_TOTAL.copy(b);
        M_DUP.copy(b);
        M_MCOUNT.copy(b);
        NID_C.copy(b);
        NID_BG.copy(b);
        Q_LINK.copy(b);
        for (auto &pack : packets) {
            pack->copy(b);
        }
        NID_PACKET_t NID_PACKET;
        NID_PACKET.rawdata = 255;
        NID_PACKET.copy(b);
    }
};
extern std::deque<std::pair<eurobalise_telegram, std::pair<distance,int64_t>>> pending_telegrams;
extern std::list<link_data>::iterator link_expected;
void update_track_comm();
void handle_radio_message(std::shared_ptr<euroradio_message> msg, communication_session *session);
void set_message_filters();