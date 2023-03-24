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
struct ETCS_message
{
    bool valid;
    bool readerror;
    virtual void write_to(bit_manipulator &b) = 0;
    virtual ~ETCS_message() {}
};
struct ETCS_packet
{
    NID_PACKET_t NID_PACKET;
    L_PACKET_t L_PACKET;
    ETCS_packet() {}
    ETCS_packet(bit_manipulator &r)
    {
        r.read(&NID_PACKET);
    }
    bool directional=false;
    virtual void copy(bit_manipulator &w) 
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        w.position += L_PACKET.rawdata - 21;
    }
    virtual void write_to(bit_manipulator &w)
    {
        int start = w.position;
        copy(w);
        L_PACKET.rawdata = w.position-start;
        w.replace(&L_PACKET, start+8);
    }
    static ETCS_packet *construct(bit_manipulator &r, int m_version);
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
    virtual void copy(bit_manipulator &w) override
    {
        int pos = w.position;
        NID_PACKET.copy(w);
        Q_DIR.copy(w);
        L_PACKET.copy(w);
        if (!w.write_mode)
            w.position = pos+L_PACKET;
    }
    void write_to(bit_manipulator &w) override
    {
        int start = w.position;
        copy(w);
        L_PACKET.rawdata = w.position-start;
        w.replace(&L_PACKET, start+10);
    }
};