#pragma once
#include <set>
#include <vector>
#include "distance.h"
#include "conversion_model.h"
#include "supervision.h"
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
public:
    target_class type;
    bool is_EBD_based() const
    {
        return type != target_class::EoA;
    }
    target() : type(target_class::MRSP) {};
    target(distance dist, double speed, target_class type) : d_target(dist), V_target(speed), type(type)
    {
    }
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
    void calculate_curves(double V_est=::V_est) const;
    bool operator< (const target t) const
    {
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
        return V_target == t.V_target && d_target==t.d_target && (int)type==(int)t.type;
    }
};
class EndOfAuthority
{
public:
    distance get_location() {return distance(1500);}
};
class SupervisionLimit
{
public:
    distance get_location() {return distance(1700);}
};
extern EndOfAuthority *EoA;
extern SupervisionLimit *SvL;
void set_supervised_targets();
std::set<target> get_supervised_targets();
bool supervised_targets_changed();
