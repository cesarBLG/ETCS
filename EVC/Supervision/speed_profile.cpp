#include "speed_profile.h"
#include "targets.h"
void speed_restriction_list::recalculate_MRSP()
{
    MRSP.clear();
    std::set<distance> critical_points;
    for (auto it = restrictions.begin(); it != restrictions.end(); ++it) {
        critical_points.insert((*it)->get_start());
        critical_points.insert((*it)->get_end());
    }
    for (auto it = critical_points.begin(); it != critical_points.end(); ++it) {
        double spd=400;
        for (auto it2 = restrictions.begin(); it2 != restrictions.end(); ++it2) {
            if ((*it2)->get_start()<=(*it) && (*it2)->get_end()>(*it) && (*it2)->get_speed()<spd)
                spd = (*it2)->get_speed();
        }
        if (MRSP.size()==0 || (--MRSP.upper_bound(*it))->second!=spd)
            MRSP[*it] = spd;
    }
    set_supervised_targets();
}
speed_restriction_list mrsp_candidates;
