#pragma once
#include "packets.h"
#include <vector>
struct LinkingElement
{
    D_LINK_t D_LINK;
    Q_NEWCOUNTRY_t Q_NEWCOUNTRY;
    NID_C_t NID_C;
    NID_BG_t NID_BG;
    Q_LINKORIENTATION_t Q_LINKORIENTATION;
    Q_LINKREACTION_t Q_LINKREACTION;
    Q_LOCACC_t Q_LOCACC;
    void copy(bit_manipulator &r)
    {
        D_LINK.copy(r);
        Q_NEWCOUNTRY.copy(r);
        if (Q_NEWCOUNTRY)
            NID_C.copy(r);
        NID_BG.copy(r);
        Q_LINKORIENTATION.copy(r);
        Q_LINKREACTION.copy(r);
        Q_LOCACC.copy(r);
    }
};
struct Linking : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    LinkingElement element;
    N_ITER_t N_ITER;
    std::vector<LinkingElement> elements;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        element.copy(r);
        N_ITER.copy(r);
        elements.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements[i].copy(r);
        }
    }
};
