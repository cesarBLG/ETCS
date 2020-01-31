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
    LinkingElement(){}
    LinkingElement(bit_read_temp &r)
    {
        r.read(&D_LINK);
        r.read(&Q_NEWCOUNTRY);
        if (Q_NEWCOUNTRY)
            r.read(&NID_C);
        r.read(&NID_BG);
        r.read(&Q_LINKORIENTATION);
        r.read(&Q_LINKREACTION);
        r.read(&Q_LOCACC);
    }
};
struct Linking : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    LinkingElement element;
    N_ITER_t N_ITER;
    std::vector<LinkingElement> elements;
    Linking() {}
    Linking(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        element = LinkingElement(r);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements.push_back(LinkingElement(r));
        }
    }
    Linking *create(bit_read_temp &r)
    {
        return new Linking(r);
    }
};