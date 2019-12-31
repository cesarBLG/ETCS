#include "speed_profile.h"
#include "targets.h"
#include "train_data.h"
#include <vector>
#include <map>
#include <list>
speed_restriction_list mrsp_candidates;
static std::vector<speed_restriction> SSP;
speed_restriction *train_speed=nullptr;
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
void update_SSP(std::vector<SSP_element> nSSP)
{
    std::vector<speed_restriction> rest;
    for (auto it=nSSP.begin(); it!=--nSSP.end(); ++it) {
        auto next = it;
        next++;
        rest.push_back(speed_restriction(it->get_speed(cant_deficiency,other_train_categories), it->start, next->start, true));
    }
    for (speed_restriction &r : SSP) {
        mrsp_candidates.remove_restriction(&r);
    }
    SSP = rest;
    for (speed_restriction &r : SSP) {
        mrsp_candidates.insert_restriction(&r);
    }
}
void set_train_max_speed(double vel)
{
    if (train_speed != nullptr) {
        mrsp_candidates.remove_restriction(train_speed);
        delete train_speed;
    }
    train_speed = new speed_restriction(V_train, ::distance(std::numeric_limits<double>::lowest()), ::distance(std::numeric_limits<double>::max()), false);
    mrsp_candidates.insert_restriction(train_speed);
}
std::list<TSR> TSRs;
void insert_TSR(TSR rest)
{
    revoke_TSR(rest.id);
    TSRs.push_back(rest);
    mrsp_candidates.insert_restriction(&TSRs.back().restriction);
}
void revoke_TSR(int id_tsr)
{
    std::list<std::list<TSR>::iterator> revocable;
    for (auto it=TSRs.begin(); it!=TSRs.end(); ++it) {
        if (it->id == id_tsr && it->revocable) {
            mrsp_candidates.remove_restriction(&it->restriction);
            revocable.push_back(it);
        }
    }
    for (auto it : revocable) {
        TSRs.erase(it);
    }
}
std::map<distance, double> gradient;