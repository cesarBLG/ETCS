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
#include <map>
struct ETCS_packet
{
    NID_PACKET_t NID_PACKET;
    L_PACKET_t L_PACKET;
    ETCS_packet() {}
    ETCS_packet(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
    }
    bool directional=false;
    virtual ETCS_packet *create(bit_read_temp &r)
    {
        return new ETCS_packet(r);
    }
    virtual void serialize(bit_write &w) {}
    static std::map<int, ETCS_packet*> packet_factory;
    static void initialize();
    static ETCS_packet *construct(bit_read_temp &r);
};
struct ETCS_nondirectional_packet : ETCS_packet
{
    ETCS_nondirectional_packet()
    {
        directional = false;
    }
};
struct ETCS_directional_packet : ETCS_packet
{
    Q_DIR_t Q_DIR;
    ETCS_directional_packet()
    {
        directional = true;
    }
    ETCS_directional_packet(bit_read_temp &r)
    {
        directional = true;
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.position = L_PACKET - L_PACKET.size - Q_DIR.size - NID_PACKET.size;
    }
    ETCS_packet *create(bit_read_temp &r) override
    {
        return new ETCS_directional_packet(r);
    }
};