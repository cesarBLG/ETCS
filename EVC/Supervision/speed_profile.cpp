/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "speed_profile.h"
#include "targets.h"
#include "track_pbd.h"
#include "train_data.h"
#include "../MA/movement_authority.h"
#include "fixed_values.h"
#include "national_values.h"
#include "../LX/level_crossing.h"
#include <vector>
#include <map>
#include <cmath>
std::map<relocable_dist_base,double,std::less<>> MRSP;
std::list<speed_restriction> SSP;
std::list<TSR> TSRs;
optional<speed_restriction> train_speed;
optional<speed_restriction> SR_speed;
optional<speed_restriction> SH_speed;
optional<speed_restriction> UN_speed;
optional<speed_restriction> OS_speed;
optional<speed_restriction> LS_speed;
optional<speed_restriction> override_speed;
optional<speed_restriction> STM_system_speed;
optional<speed_restriction> STM_max_speed;
struct gradient_profile_element
{
    distance start;
    distance end;
    double grad;
};
std::list<gradient_profile_element> gradient_profile;
std::map<dist_base, double> gradient;
int default_gradient_tsr;
void delete_back_info()
{
    const dist_base mindist = d_minsafefront(confidence_data::basic())-L_TRAIN-D_keep_information;
    for (auto it = SSP.begin(); it != SSP.end(); ) {
        if (it->get_end() < mindist)
            it = SSP.erase(it);
        else
            ++it;
    }
    for (auto it = gradient_profile.begin(); it != gradient_profile.end(); ) {
        if (it->end.min < mindist)
            it = gradient_profile.erase(it);
        else
            ++it;
    }
    TSRs.remove_if([mindist](const TSR &t) {
        return t.restriction.get_end()<mindist;
    });
    PBDs.remove_if([mindist](std::shared_ptr<PBD_target> &t) {
        return t->end.min < mindist;
    });
    level_crossings.remove_if([mindist](const level_crossing &lx) {
        return lx.start.min + lx.length < mindist;
    });
}
void delete_SSP(const distance &start)
{
    for (auto it = SSP.begin(); it != SSP.end(); ) {
        if (it->get_start() > start.min) {
            it = SSP.erase(it);
            continue;
        }
        if (it->get_uncompensated_end() > start.min) {
            *it = speed_restriction(it->get_speed(), it->start_distance, start, it->is_compensated());
        }
        ++it;
    }
}
void delete_SSP()
{
    SSP.clear();
}
void delete_gradient(const distance &start)
{
    for (auto it = gradient_profile.begin(); it != gradient_profile.end(); ) {
        if (it->start.max > start.min) {
            it = gradient_profile.erase(it);
            continue;
        }
        if (it->end.min > start.min) {
            *it = {it->start, start, it->grad};
        }
        ++it;
    }
}
void delete_gradient()
{
    gradient_profile.clear();
}
void delete_TSR(const distance &d)
{
    TSRs.remove_if([d](const TSR& t) {return d.min < t.restriction.get_start();});
}
void delete_TSR()
{
    TSRs.clear();
}
void delete_track_info()
{
    SSP.clear();
    gradient_profile.clear();
    TSRs.clear();
    recalculate_MRSP();
}
void recalculate_MRSP()
{
    delete_back_info();
    recalculate_gradient();
    MRSP.clear();
    std::list<std::reference_wrapper<speed_restriction>> restrictions;
    if (mode == Mode::FS || mode == Mode::OS || mode == Mode::LS)
        restrictions.insert(restrictions.end(), SSP.begin(), SSP.end());
    if (mode == Mode::FS || mode == Mode::OS || mode == Mode::LS || mode == Mode::SR || mode == Mode::UN) {
        for (auto &tsr : TSRs)
            restrictions.push_back(tsr.restriction);
    }
    if (mode == Mode::FS || mode == Mode::OS || mode == Mode::LS) {
        for (auto it=level_crossings.begin(); it!=level_crossings.end(); ++it) {
            if (!it->lx_protected && it->svl_replaced) restrictions.push_back(*it->svl_replaced);
        }
    }
    if (mode == Mode::FS || mode == Mode::OS || mode == Mode::LS) {
        for (auto &pbd : PBDs) {
            restrictions.push_back(pbd->restriction);
        }
    }
    if (train_speed && 
        (mode == Mode::FS || mode == Mode::OS || mode == Mode::LS || mode == Mode::SR || mode == Mode::UN || mode == Mode::RV))
        restrictions.push_back(*train_speed);
    if (SR_speed && mode == Mode::SR)
        restrictions.push_back(*SR_speed);
    if (OS_speed && mode == Mode::OS)
        restrictions.push_back(*OS_speed);
    if (LS_speed && mode == Mode::LS)
        restrictions.push_back(*LS_speed);
    if (SH_speed && mode == Mode::SH)
        restrictions.push_back(*SH_speed);
    if (UN_speed && mode == Mode::UN)
        restrictions.push_back(*UN_speed);
    if (STM_system_speed && (mode == Mode::FS || mode == Mode::LS || mode == Mode::OS || mode == Mode::SR || mode == Mode::UN))
        restrictions.push_back(*STM_system_speed);
    if (STM_max_speed && (mode == Mode::FS || mode == Mode::LS || mode == Mode::OS || mode == Mode::SR || mode == Mode::UN || mode == Mode::SN))
        restrictions.push_back(*STM_max_speed);
    if (override_speed && (mode == Mode::SH || mode == Mode::SR || mode == Mode::UN))
        restrictions.push_back(*override_speed);
    if (mode == Mode::FS || mode == Mode::OS || mode == Mode::LS)
        restrictions.insert(restrictions.begin(), signal_speeds.begin(), signal_speeds.end());
    if (restrictions.empty()) {
        set_supervised_targets();
        return;
    }
    std::set<relocable_dist_base> critical_points;
    for (auto it = restrictions.begin(); it != restrictions.end(); ++it) {
        critical_points.insert(it->get().get_start());
        critical_points.insert(it->get().get_end());
    }
    for (auto it = critical_points.begin(); it != --critical_points.end(); ++it) {
        double spd=400;
        for (auto it2 = restrictions.begin(); it2 != restrictions.end(); ++it2) {
            if (it2->get().get_start()<=*it && it2->get().get_end()>*it && it2->get().get_speed()<spd)
                spd = it2->get().get_speed();
        }
        if (MRSP.size()==0 || (--MRSP.upper_bound(*it))->second!=spd)
            MRSP[*it] = spd;
    }
    set_supervised_targets();
}
std::map<relocable_dist_base,double,std::less<>> &get_MRSP()
{
    return MRSP;
}
void update_SSP(std::vector<SSP_element> nSSP)
{
    distance start = nSSP[0].start;
    std::list<speed_restriction> rest;
    for (auto it=nSSP.begin(); it!=nSSP.end(); ++it) {
        auto next = it;
        if (it->restrictions[0][0]<0) break;
        ++next;
        distance end = next==nSSP.end() ? distance::from_odometer(dist_base::max) : next->start;
        rest.push_back(speed_restriction(it->get_speed(cant_deficiency,other_train_categories), it->start, end, it->compensate_train_length));
    }
    delete_SSP(start);
    SSP.insert(SSP.end(), rest.begin(), rest.end());
}
dist_base SSP_begin()
{
    dist_base d = dist_base::max;
    for (auto it = SSP.begin(); it != SSP.end(); ++it) {
        auto &c = it->start_distance.est;
        if (c < d)
            d = c;
    }
    return d;
}
dist_base SSP_end()
{
    dist_base d = dist_base::min;
    for (auto it = SSP.begin(); it != SSP.end(); ++it) {
        auto &c = it->end_distance.max;
        if (c > d)
            d = c;
    }
    return d;
}
void update_gradient(std::vector<std::pair<distance,double>> grad)
{
    distance start = grad[0].first;
    std::list<gradient_profile_element> prof;
    for (auto it=grad.begin(); it!=grad.end(); ++it) {
        auto next = it;
        if (it->second == 1000) break;
        ++next;
        distance end = next==grad.end() ? distance::from_odometer(dist_base::max) : next->first;
        prof.push_back({it->first, end, it->second});
    }
    delete_gradient(start);
    gradient_profile.insert(gradient_profile.end(), prof.begin(), prof.end());
    recalculate_gradient();
}
void recalculate_gradient()
{
    gradient.clear();
    std::set<dist_base> critical_points;
    for (auto it = gradient_profile.begin(); it != gradient_profile.end(); ++it) {
        critical_points.insert(it->start.max);
        critical_points.insert(it->end.min);
    }
    for (auto &d : critical_points) {
        double grad = 1000;
        for (auto it = gradient_profile.begin(); it != gradient_profile.end(); ++it) {
            if (it->start.max <= d && it->end.min > d && it->grad < grad)
                grad = it->grad;
        }
        gradient[d] = grad;
    }
}
const std::map<dist_base, double> &get_gradient()
{
    return gradient;
}
void set_train_max_speed(double vel)
{
    V_train = vel;
    train_speed = speed_restriction(V_train, distance::from_odometer(dist_base::min), distance::from_odometer(dist_base::max), false);
}
bool inhibit_revocable_tsr;
void insert_TSR(TSR rest)
{
    revoke_TSR(rest.id);
    TSRs.push_back(rest);
}
void revoke_TSR(int id_tsr)
{
    std::list<std::list<TSR>::iterator> revocable;
    for (auto it=TSRs.begin(); it!=TSRs.end(); ++it) {
        if (it->id == id_tsr && it->revocable) {
            revocable.push_back(it);
        }
    }
    for (auto it : revocable) {
        TSRs.erase(it);
    }
}
speed_restriction get_PBD_restriction(double d_PBD, distance start, distance end, bool EB, double g)
{
    double V_pbd=0;
    double V_test = 1;
    double V_max_appr = std::max(std::sqrt(2*2*d_PBD),600/3.6);
    PBD_target pbd_ebd(start, end, d_PBD, true, g);
    if (EB) {
        while (V_test<V_max_appr) {
            double dvebi = dV_ebi(V_test);
            double V_delta0PBD = Q_NVINHSMICPERM ? 0 : (V_test+dvebi)*0.008;
            double D_bec = (V_test+dvebi+V_delta0PBD)*(pbd_ebd.T_traction+pbd_ebd.T_berem);
            double d_offset=L_antenna_front+0.001*(V_test+dvebi+V_delta0PBD);
            double V_ebd = pbd_ebd.get_speed_curve(start.max+(d_offset+D_bec));
            if (std::abs((V_test+dvebi)-(V_ebd-V_delta0PBD))<=1.0/3.6 && d_offset+D_bec<d_PBD) {
                V_pbd = V_test;
                break;
            }
            V_test+=0.5;
        }
    } else {

    }
    return speed_restriction((((int)(V_pbd*3.6))/5)*5/3.6, start, end, false);
}
void delete_PBD()
{
    PBDs.clear();
}
void delete_PBD(const distance &from)
{
    //PBDs.remove_if([from](auto &pbd) { return pbd->end > from.min; });
}