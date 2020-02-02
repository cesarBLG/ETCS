#pragma once
#include <set>
#include <vector>
#include <optional>
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
    distance d_target;
    double V_target;
    bool is_valid;
public:
    target_class type;
    bool is_EBD_based;
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
    void calculate_decelerations();
    void calculate_decelerations(std::map<distance,double> gradient);
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
class PBD_target : public target
{
    bool emergency;
    public:
    PBD_target(distance d_PBD, bool emergency, double grad) : target(d_PBD, 0, target_class::PBD)
    {
        std::map<distance,double> gradient;
        gradient[distance(std::numeric_limits<double>::lowest())] = grad;
        is_EBD_based = emergency;
        calculate_decelerations(gradient);
    }
};
extern std::optional<distance> EoA;
extern std::optional<distance> SvL;
extern std::optional<distance> SR_dist;
extern std::optional<std::pair<distance,double>> LoA;
extern double V_releaseSvL;
void set_supervised_targets();
std::set<target> get_supervised_targets();
bool supervised_targets_changed();
