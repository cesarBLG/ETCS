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
#include "../Position/linking.h"
#include "../MA/movement_authority.h"
#include "../Supervision/speed_profile.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <deque>
struct eurobalise_telegram
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
    bool valid;
    bool readerror;
    eurobalise_telegram(bit_read_temp &r)
    {
        r.read(&Q_UPDOWN);
        r.read(&M_VERSION);
        r.read(&Q_MEDIA);
        r.read(&N_PIG);
        r.read(&N_TOTAL);
        r.read(&M_DUP);
        r.read(&M_MCOUNT);
        r.read(&NID_C);
        r.read(&NID_BG);
        r.read(&Q_LINK);
        while (!r.error)
        {
            NID_PACKET_t NID_PACKET;
            r.peek(&NID_PACKET);
            if (NID_PACKET==255)
                break;
            packets.push_back(std::shared_ptr<ETCS_packet>(ETCS_packet::construct(r)));
        }
        readerror = r.error;
        valid = !r.sparefound;
    }
};
struct message_packet
{
    ETCS_packet *p;
    distance dist;
    bool infill;
    int dir;
    bool fromRBC;
};
extern std::deque<eurobalise_telegram> pending_telegrams;
extern std::list<link_data>::iterator link_expected;
void check_eurobalise_passed();
void set_message_filters();