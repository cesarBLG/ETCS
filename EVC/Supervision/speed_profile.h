/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <set>
#include <map>
#include <list>
#include <vector>
#include "../optional.h"
#include "../Position/distance.h"
#include "../SSP/ssp.h"
#include "fixed_values.h"
#include "train_data.h"
#include "../Version/version.h"
void recalculate_MRSP();
void recalculate_gradient();
void delete_track_info();
void delete_track_info(const distance &from);
void delete_SSP();
void delete_SSP(const distance &from);
void delete_ASP();
void delete_ASP(const distance &from);
void delete_gradient();
void delete_gradient(const distance &from);
void delete_TSR();
void delete_TSR(const distance &from);
void delete_PBD();
void delete_PBD(const distance &from);
std::map<relocable_dist_base,double,std::less<>> &get_MRSP();
inline double dV_ebi(double vel)
{
    return std::max(dV_ebi_min, std::min(dV_ebi_min*(dV_ebi_max - dV_ebi_min)/(V_ebi_max-V_ebi_min)*(vel-V_ebi_min), dV_ebi_max));
}
inline double dV_sbi(double vel)
{
    return std::max(dV_sbi_min, std::min(dV_sbi_min*(dV_sbi_max - dV_sbi_min)/(V_sbi_max-V_sbi_min)*(vel-V_sbi_min), dV_sbi_max));
}
inline double dV_warning(double vel)
{
    return std::max(dV_warning_min, std::min(dV_warning_min*(dV_warning_max - dV_warning_min)/(V_warning_max-V_warning_min)*(vel-V_warning_min), dV_warning_max));
}
class speed_restriction
{
public:
    double speed;
    distance start_distance;
    distance end_distance;
    bool compensate_train_length;
    bool is_tsr=false;
    speed_restriction(double spd, distance start, distance end, bool compensate_trainlength) : speed(spd), start_distance(start), end_distance(end), compensate_train_length(compensate_trainlength) {}
    double get_speed() const { return speed; }
    relocable_dist_base get_start() const
    {
#if BASELINE == 4
        if (VERSION_X(operated_version) < 3 && is_tsr)
            return start_distance.est;
#endif
        return start_distance.max;
    }
    relocable_dist_base get_end() const
    {
#if BASELINE == 4
        if (VERSION_X(operated_version) < 3 && is_tsr)
            return end_distance.est + compensate_train_length*L_TRAIN;
#endif
        return end_distance.min + compensate_train_length*L_TRAIN;
    }
    relocable_dist_base get_uncompensated_end() const
    {
#if BASELINE == 4
        if (VERSION_X(operated_version) < 3 && is_tsr)
            return end_distance.est;
#endif
        return end_distance.min;
    }
    bool is_compensated() const
    {
        return compensate_train_length;
    }
};
void set_train_max_speed(double vel);
void update_SSP(std::vector<SSP_element> nSSP);
dist_base SSP_begin();
dist_base SSP_end();
void update_ASP(distance start, std::vector<speed_restriction> &nASP);
void update_gradient(std::vector<std::pair<distance,double>> grad);
const std::map<dist_base, double> &get_gradient();
extern int default_gradient_tsr;
struct TSR
{
    int id;
    bool revocable;
    speed_restriction restriction;
};
void insert_TSR(TSR rest);
void revoke_TSR(int id_tsr);
extern bool inhibit_revocable_tsr;
extern std::list<TSR> TSRs;
extern optional<speed_restriction> SR_speed;
extern optional<speed_restriction> SH_speed;
extern optional<speed_restriction> UN_speed;
extern optional<speed_restriction> OS_speed;
extern optional<speed_restriction> LS_speed;
extern optional<speed_restriction> RV_speed;
extern optional<speed_restriction> STM_system_speed;
extern optional<speed_restriction> STM_max_speed;
extern optional<speed_restriction> override_speed;
speed_restriction get_PBD_restriction(double d_PBD, distance start, distance end, bool EB, double g);
