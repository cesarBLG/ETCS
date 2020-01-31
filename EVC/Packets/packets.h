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
    static std::map<int, ETCS_packet*> packet_factory;
    static void initialize();
    static ETCS_packet *construct(bit_read_temp &r)
    {
        int pos = r.position;
        NID_PACKET_t NID_PACKET;
        r.peek(&NID_PACKET);
        ETCS_packet *p;
        if (packet_factory.find(NID_PACKET) == packet_factory.end()) {
            r.sparefound = true;
            p = new ETCS_packet(r);
        } else {
            p = packet_factory[NID_PACKET]->create(r);
        }
        if (r.position-pos != p->L_PACKET)
            r.error = true;
        return p;
    }
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
};