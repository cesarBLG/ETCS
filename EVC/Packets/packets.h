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
    virtual ~ETCS_packet() = default;
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