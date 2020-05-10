#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct TSR_element_packet
{
    NID_TSR_t NID_TSR;
    D_TSR_t D_TSR;
    L_TSR_t L_TSR;
    Q_FRONT_t Q_FRONT;
    V_TSR_t V_TSR;
    TSR_element_packet() = default;
    TSR_element_packet(bit_read_temp &r)
    {
        r.read(&NID_TSR);
        r.read(&D_TSR);
        r.read(&L_TSR);
        r.read(&Q_FRONT);
        r.read(&V_TSR);
    }
};

struct TemporarySpeedRestriction : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    TSR_element_packet element;
    N_ITER_t N_ITER;
    std::vector<TSR_element_packet> elements;
    TemporarySpeedRestriction() = default;
    TemporarySpeedRestriction(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        element = TSR_element_packet(r);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements.push_back(TSR_element_packet(r));
        }
    }
    TemporarySpeedRestriction *create(bit_read_temp &r) override
    {
        return new TemporarySpeedRestriction(r);
    }
};