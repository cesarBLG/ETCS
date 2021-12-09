/*
 * European Train Control System
 * Copyright (C) 2022  César Benito <cesarbema2009@hotmail.com>
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
#include <vector>
#include <map>
#include <memory>
#include "variables.h"
#include "types.h"
#include "packets.h"
struct euroradio_message
{
    NID_MESSAGE_t NID_MESSAGE;
    L_MESSAGE_t L_MESSAGE;
    T_TRAIN_t T_TRAIN;
    M_ACK_t M_ACK;
    NID_LRBG_t NID_LRBG;
    static std::map<int, euroradio_message*> message_factory;
    std::vector<std::shared_ptr<ETCS_packet>> packets;
    std::vector<std::shared_ptr<ETCS_packet>> optional_packets;
    bool valid;
    bool readerror;
    euroradio_message() {}
    euroradio_message(bit_read_temp &r)
    {
        r.read(&NID_MESSAGE);
        r.read(&L_MESSAGE);
        r.read(&T_TRAIN);
        r.read(&M_ACK);
        r.read(&NID_LRBG);
    }
    static void initialize();
    virtual euroradio_message* create(bit_read_temp &r) {return new euroradio_message(r);}
    static std::shared_ptr<euroradio_message> build(unsigned char* data, int size)
    {
        bit_read_temp r(data, size);
        build(r);
    }
    static std::shared_ptr<euroradio_message> build(bit_read_temp &r)
    {
        int size = r.bits.size()/8;
        NID_MESSAGE_t nid;
        r.peek(&nid);
        std::shared_ptr<euroradio_message> msg;
        if (message_factory.find(nid) == message_factory.end()) {
            r.sparefound = true;
            msg = std::shared_ptr<euroradio_message>(new euroradio_message(r));
        } else {
            msg = std::shared_ptr<euroradio_message>(message_factory[nid]->create(r));
        }
        while (!r.error && r.position<=r.bits.size()-8)
        {
            NID_PACKET_t NID_PACKET;
            r.peek(&NID_PACKET);
            if (NID_PACKET==255)
                break;
            msg->optional_packets.push_back(std::shared_ptr<ETCS_packet>(ETCS_packet::construct(r)));
        }
        msg->packets.insert(msg->packets.end(), msg->optional_packets.begin(), msg->optional_packets.end());
        if (msg->L_MESSAGE != size) r.error=true;
        msg->readerror = r.error;
        msg->valid = !r.sparefound;
        return msg;
    }
};