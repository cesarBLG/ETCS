#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct RouteSuitability_element
{
    D_SUITABILITY_t D_SUITABILITY;
    Q_SUITABILITY_t Q_SUITABILITY;
    M_LINEGAUGE_t M_LINEGAUGE;
    M_AXLELOADCAT_t M_AXLELOADCAT;
    M_VOLTAGE_t M_VOLTAGE;
    NID_CTRACTION_t NID_CTRACTION;
    void copy(bit_manipulator &r)
    {
        D_SUITABILITY.copy(r);
        Q_SUITABILITY.copy(r);
        if (Q_SUITABILITY.rawdata == Q_SUITABILITY.LoadingGauge) M_LINEGAUGE.copy(r);
        else if (Q_SUITABILITY.rawdata == Q_SUITABILITY.MaxAxleLoad) M_AXLELOADCAT.copy(r);
        else if (Q_SUITABILITY.rawdata == Q_SUITABILITY.TractionSystem) {
            M_VOLTAGE.copy(r);
            if (M_VOLTAGE != 0) NID_CTRACTION.copy(r);
        }

    }
};
struct RouteSuitabilityData : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    Q_TRACKINIT_t Q_TRACKINIT;
    D_TRACKINIT_t D_TRACKINIT;
    RouteSuitability_element element;
    N_ITER_t N_ITER;
    std::vector<RouteSuitability_element> elements;
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