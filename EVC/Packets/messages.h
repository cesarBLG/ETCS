/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
        b.m_version = M_VERSION.rawdata;
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
            packets.push_back(std::shared_ptr<ETCS_packet>(ETCS_packet::construct(b, M_VERSION)));
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
bool handle_radio_message(std::shared_ptr<euroradio_message> msg, communication_session *session);
void set_message_filters();