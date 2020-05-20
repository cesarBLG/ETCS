#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct GeographicalPosition_element_packet
{
    Q_NEWCOUNTRY_t Q_NEWCOUNTRY;
    NID_C_t NID_C;
    NID_BG_t NID_BG;
    D_POSOFF_t D_POSOFF;
    Q_MPOSITION_t Q_MPOSITION;
    M_POSITION_t M_POSITION;

    GeographicalPosition_element_packet() = default;
    GeographicalPosition_element_packet(bit_read_temp &r)
    {
        r.read(&Q_NEWCOUNTRY);
        if (Q_NEWCOUNTRY)
            r.read(&NID_C);
        r.read(&NID_BG);
        r.read(&D_POSOFF);
        r.read(&Q_MPOSITION);
        r.read(&M_POSITION);
    }
};

struct GeographicalPosition : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    GeographicalPosition_element_packet element;
    N_ITER_t N_ITER;
    std::vector<GeographicalPosition_element_packet> elements;
    GeographicalPosition() = default;
    GeographicalPosition(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);

        element = GeographicalPosition_element_packet(r);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements.push_back(GeographicalPosition_element_packet(r));
        }
    }
    GeographicalPosition *create(bit_read_temp &r) override
    {
        return new GeographicalPosition(r);
    }
};