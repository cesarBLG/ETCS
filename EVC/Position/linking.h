#pragma once
#include "distance.h"
#include "../Packets/5.h"
#include <list>
struct bg_id
{
    int NID_C;
    int NID_BG;
    bool operator==(const bg_id &o) const
    {
        return NID_C == o.NID_C && NID_BG == o.NID_BG;
    }
};
struct link_data
{
    bg_id nid_bg;
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
extern bg_id NID_LRBG;
extern double Q_LOCACC_LRBG;
double update_location_reference(bg_id nid_bg, distance group_pos, bool linked=true);
void update_linking(distance start, Linking link, bool infill, bg_id this_bg);
void delete_linking();