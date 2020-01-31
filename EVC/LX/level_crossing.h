#pragma once
#include "../Position/distance.h"
#include <set>
class level_crossing
{
    int id;
    distance start;
    double length;
    bool lx_protected;
    double V_LX;
    bool stop;
    double stoplength;
    bool operator<(const level_crossing l) const
    {
        return start<l.start;
    }
};
extern std::set<level_crossing> level_crossings;