#pragma once
#include <set>
#include <vector>
#include "acceleration.h"
#include "distance.h"
#include "supervision.h"
#include "conversion_model.h"
enum struct target_class
{
    EoA,
    LoA,
    MRSP,
    SvL,
    SR_distance
};
class target
{
    distance d_target;
    double V_target;
    bool is_valid;
public:
    target_class type;
    bool is_EBD_based() const
    {
        return type != target_class::EoA;
    }
    target();
    target(distance dist, double speed, target_class type);
    double get_target_speed() const { return V_target; }
    distance get_target_position() const { return d_target; }
    distance get_distance_curve (double velocity) const;
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
    void calculate_curves(double V_est=::V_est) const;
    void calculate_decelerations();
    bool operator< (const target t) const
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
    bool operator== (const target t) const
    {
        if (!is_valid || !t.is_valid)
            return false;
        return V_target == t.V_target && d_target==t.d_target && (int)type==(int)t.type;
    }
};
extern distance *EoA;
extern distance *SvL;
extern double V_releaseSvL;
void set_supervised_targets();
std::set<target> get_supervised_targets();
bool supervised_targets_changed();
