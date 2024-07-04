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
#include <vector>
#include <list>
#include <variant>
#include "../optional.h"
#include "acceleration.h"
#include "../Position/distance.h"
#include "supervision.h"
#include "conversion_model.h"
enum struct target_class
{
    EoA,
    LoA,
    MRSP,
    SvL,
    SR_distance,
    PBD
};
class basic_target
{
protected:
    relocable_dist_base d_target;
    double V_target;
public:
    target_class type;
    bool is_EBD_based;
    bool is_TSR = false;
    basic_target() {}
    basic_target(relocable_dist_base dist, double speed, target_class type, bool is_TSR) : d_target(dist), V_target(speed), type(type), is_TSR(is_TSR)
    {
        is_EBD_based = type != target_class::EoA;
    }
    double get_target_speed() const { return V_target; }
    relocable_dist_base get_target_position() const { return d_target; }
    bool operator== (const basic_target &t) const
    {
        return V_target == t.V_target && std::abs(d_target-t.d_target)<2.1f && (int)type==(int)t.type;
    }
};
class target : public basic_target
{
protected:
    bool use_brake_combination = true;
public:
    double default_gradient=0;
    target(relocable_dist_base dist, double speed, target_class type, bool is_tsr=false);
    virtual dist_base get_distance_curve (double velocity) const;
    double get_speed_curve(dist_base dist) const;
    dist_base get_distance_gui_curve(double velocity) const;
    double get_speed_gui_curve(dist_base dist) const;
    mutable dist_base d_EBI;
    mutable dist_base d_SBI2;
    mutable dist_base d_SBI1;
    mutable dist_base d_W;
    mutable dist_base d_P;
    mutable dist_base d_I;
    mutable double V_SBI2;
    mutable double V_SBI1;
    mutable double V_P;
    mutable acceleration A_safe;
    mutable acceleration A_expected;
    mutable acceleration A_normal_service;
    mutable double A_est1;
    mutable double A_est2;
    mutable double T_traction;
    mutable double T_berem;
    mutable double T_brake_emergency;
    mutable double T_brake_service;
    mutable double T_be;
    mutable double T_bs;
    mutable double T_bs1;
    mutable double T_bs2;
    virtual void calculate_times() const;
    void calculate_curves(double V_est=::V_est, double A_est=::A_est, double V_delta=::V_ura) const;
    virtual void calculate_decelerations();
    void calculate_decelerations(const std::map<dist_base,double> &gradient);
    static void recalculate_all_decelerations();
};
extern optional<distance> EoA;
extern optional<distance> SvL;
extern optional<distance> SR_dist;
extern optional<double> D_STFF_rbc;
extern optional<std::pair<distance,double>> LoA;
extern double V_releaseSvL;
void set_supervised_targets();
const std::list<std::shared_ptr<target>> &get_supervised_targets();
bool supervised_targets_changed();