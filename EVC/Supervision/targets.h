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
class target
{
protected:
    distance d_target;
    double V_target;
    bool is_valid;
    bool use_brake_combination = true;
public:
    target_class type;
    bool is_EBD_based;
    bool is_TSR = false;
    double default_gradient=0;
    target();
    target(distance dist, double speed, target_class type);
    double get_target_speed() const { return V_target; }
    distance get_target_position() const { return d_target; }
    virtual distance get_distance_curve (double velocity) const;
    double get_speed_curve(distance dist) const;
    distance get_distance_gui_curve(double velocity) const;
    double get_speed_gui_curve(distance dist) const;
    mutable distance d_EBI;
    mutable distance d_SBI2;
    mutable distance d_SBI1;
    mutable distance d_W;
    mutable distance d_P;
    mutable distance d_I;
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
    void calculate_times() const;
    void calculate_curves(double V_est=::V_est, double A_est=::A_est, double V_delta=::V_ura) const;
    virtual void calculate_decelerations();
    void calculate_decelerations(const std::map<distance,double> &gradient);
    bool operator< (const target &t) const
    {
        if (!is_valid)
            return t.is_valid;
        if (!t.is_valid)
            return !is_valid;
        if (d_target == t.d_target) {
            if (V_target == t.V_target) {
                return (int)type<(int)t.type;
            }
            return V_target<t.V_target;
        }
        return d_target<t.d_target;
    }
    bool operator== (const target &t) const
    {
        if (!is_valid || !t.is_valid)
            return false;
        return V_target == t.V_target && std::abs(d_target-t.d_target)<1.1f && (int)type==(int)t.type;
    }
    static void recalculate_all_decelerations();
};
extern optional<distance> EoA;
extern optional<distance> SvL;
extern optional<distance> SR_dist;
extern optional<double> D_STFF_rbc;
extern optional<std::pair<distance,double>> LoA;
extern double V_releaseSvL;
void set_supervised_targets();
const std::list<target> &get_supervised_targets();
bool supervised_targets_changed();