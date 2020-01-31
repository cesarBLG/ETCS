#pragma once
#include "distance.h"
#include "../Packets/5.h"
#include <list>
struct link_data
{
    int nid_bg;
    distance dist;
    double locacc;
    bool reverse_dir;
    int reaction;
    distance max()
    {
        return dist+locacc;
    }
    distance min()
    {
        return dist-locacc;
    }
};
extern std::list<link_data> linking;
extern int NID_LRBG;
extern double Q_LOCACC_LRBG;
double update_location_reference(int nid_bg, distance group_pos, bool linked=true);
void update_linking(distance start, Linking link, bool infill);