#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct TC_station_element_packet
{
    D_TRACKCOND_t D_TRACKCOND;
    L_TRACKCOND_t L_TRACKCOND;
    M_PLATFORM_t M_PLATFORM;
    Q_PLATFORM_t Q_PLATFORM;
    void copy(bit_manipulator &r)
    {
        D_TRACKCOND.copy(r);
        L_TRACKCOND.copy(r);
        M_PLATFORM.copy(r);
        Q_PLATFORM.copy(r);
    }
};

struct TrackConditionStationPlatforms : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    TC_station_element_packet element;
    N_ITER_t N_ITER;
    Q_TRACKINIT_t Q_TRACKINIT;
    D_TRACKINIT_t D_TRACKINIT;
    std::vector<TC_station_element_packet> elements;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        Q_TRACKINIT.copy(r);
        if (Q_TRACKINIT == 1) {
            D_TRACKINIT.copy(r);
        } else {      
            element.copy(r);
            N_ITER.copy(r);
            elements.resize(N_ITER);
            for (int i=0; i<N_ITER; i++) {
                elements[i].copy(r);
            }
        } 
    }
};