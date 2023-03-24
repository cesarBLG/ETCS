#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct PBD_element
{
    D_PBD_t D_PBD;
    Q_GDIR_t Q_GDIR;
    G_PBDSR_t G_PBDSR;
    Q_PBDSR_t Q_PBDSR;
    D_PBDSR_t D_PBDSR;
    L_PBDSR_t L_PBDSR;
    void copy(bit_manipulator &r)
    {
        D_PBD.copy(r);
        Q_GDIR.copy(r);
        G_PBDSR.copy(r);
        Q_PBDSR.copy(r);
        D_PBDSR.copy(r);
        L_PBDSR.copy(r);
    }
};
struct PermittedBrakingDistanceInformation : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    Q_TRACKINIT_t Q_TRACKINIT;
    D_TRACKINIT_t D_TRACKINIT;
    PBD_element element;
    N_ITER_t N_ITER;
    std::vector<PBD_element> elements;
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