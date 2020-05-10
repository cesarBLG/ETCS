#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct MP_element_packet
{
    D_MAMODE_t D_MAMODE;
    M_MAMODE_t M_MAMODE;
    V_MAMODE_t V_MAMODE;
    L_MAMODE_t L_MAMODE;
    L_ACKMAMODE_t L_ACKMAMODE;
    Q_MAMODE_t Q_MAMODE;

    MP_element_packet() = default;
    MP_element_packet(bit_read_temp &r)
    {
        r.read(&D_MAMODE);
        r.read(&M_MAMODE);
        r.read(&V_MAMODE);
        r.read(&L_MAMODE);
        r.read(&L_ACKMAMODE);
        r.read(&Q_MAMODE);
    }
};

struct ModeProfile : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    MP_element_packet element;
    N_ITER_t N_ITER;
    std::vector<MP_element_packet> elements;
    ModeProfile() = default;
    ModeProfile(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);

        element = MP_element_packet(r);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements.push_back(MP_element_packet(r));
        }

    }
    ModeProfile *create(bit_read_temp &r) override
    {
        return new ModeProfile(r);
    }
};