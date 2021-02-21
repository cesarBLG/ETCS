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
    
    TC_station_element_packet() = default;
    TC_station_element_packet(bit_read_temp &r)
    {
        r.read(&D_TRACKCOND);
        r.read(&L_TRACKCOND);
        r.read(&M_PLATFORM);
        r.read(&Q_PLATFORM);
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
    TrackConditionStationPlatforms() = default;
    TrackConditionStationPlatforms(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        r.read(&Q_TRACKINIT);
        if (Q_TRACKINIT == 1) {
            r.read(&D_TRACKINIT);
        } else {      
            element = TC_station_element_packet(r);
            r.read(&N_ITER);
            for (int i=0; i<N_ITER; i++) {
                elements.push_back(TC_station_element_packet(r));
            }
        } 
    }
    TrackConditionStationPlatforms *create(bit_read_temp &r) override
    {
        return new TrackConditionStationPlatforms(r);
    }
};