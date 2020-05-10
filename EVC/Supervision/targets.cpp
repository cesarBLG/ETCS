#include "targets.h"
#include "curve_calc.h"
#include "speed_profile.h"
#include "conversion_model.h"
#include "supervision.h"
#include "national_values.h"
#include "fixed_values.h"
#include "train_data.h"
#include <set>
#include <iostream>
target::target() : is_valid(false), type(target_class::MRSP) {};
target::target(distance dist, double speed, target_class type) : d_target(dist), V_target(speed), is_valid(true), type(type) 
{
    calculate_decelerations();
    is_EBD_based = type != target_class::EoA;
}
distance target::get_distance_curve(double velocity) const
{
    /*if (type == target_class::MRSP) {
        distance a = distance_curve(A_safe, d_target, V_target+dV_ebi(V_target), velocity).get();
        double v = V_target+dV_ebi(V_target);
        distance x = d_target;
        bool inc = velocity > v;
        double dt = 0.01;
        while(inc ? (v < velocity) : (v > velocity)) {
            double pv = v;
            v += (inc ? 1 : -1)*A_safe.accel(v,x)*dt;
            x += (inc ? -1 : 1)*dt*0.5*(pv + v + A_safe.accel(pv,x)*dt);
        }
        double diff = std::abs(x-a);
        if (diff > 2)
            std::cout<<diff<<std::endl;
        return a;
    }*/
    if (is_EBD_based) {
        if (type == target_class::SvL || type == target_class::SR_distance || type == target_class::PBD)
            return distance_curve(A_safe, d_target, 0, velocity);
        else
            return distance_curve(A_safe, d_target, V_target+dV_ebi(V_target), velocity);
    } else {
        return distance_curve(A_expected, d_target, 0, velocity);
    }
}
double target::get_speed_curve(distance dist) const
{
    if (is_EBD_based) {
        if (type == target_class::SvL || type == target_class::SR_distance || type == target_class::PBD)
            return speed_curve(A_safe, d_target, 0, dist);
        else
            return speed_curve(A_safe, d_target, V_target+dV_ebi(V_target), dist);
    } else {
        return speed_curve(A_expected, d_target, 0, dist);
    }
}
distance target::get_distance_gui_curve(double velocity) const
{
    distance guifoot(0);
    if (type == target_class::EoA || type == target_class::SvL) {
        guifoot = d_target;
    } else {
        double V_delta0t = 0;
        distance debi = get_distance_curve(V_target+V_delta0t)-(V_target+V_delta0t)*(T_berem+T_traction);
        guifoot = debi-V_target*(T_driver+T_bs2);
    }
    return distance_curve(A_normal_service, guifoot, V_target, velocity);
}
double target::get_speed_gui_curve(distance dist) const
{
    distance guifoot(0);
    if (type == target_class::EoA || type == target_class::SvL) {
        guifoot = d_target;
    } else {
        double V_delta0t = 0.007*V_target;
        distance debi = get_distance_curve(V_target+V_delta0t)-(V_target+V_delta0t)*(T_berem+T_traction);
        guifoot = debi-V_target*(T_driver+T_bs2);
    }
    return speed_curve(A_normal_service, guifoot, V_target, dist);
}
void target::calculate_times() const
{
    if (V_target > 0) {
        T_brake_emergency = T_brake_emergency_cmt;
        T_brake_service = T_brake_service_cmt;
    } else {
        T_brake_emergency = T_brake_emergency_cm0;
        T_brake_service = T_brake_service_cm0;
    }
    T_be = (conversion_model_used ? Kt_int : 1)*T_brake_emergency;
    T_bs = T_brake_service;
    if (Q_NVSBFBPERM) {
        T_bs1 = ::T_bs1;
        T_bs2 = ::T_bs2;
    } else {
        T_bs1 = T_bs2 = T_bs;
    }
    T_traction = T_traction_cutoff;
    T_berem = std::max(0.0, T_be-T_traction);
}
void target::calculate_curves(double V_est, double A_est, double V_delta) const
{
    calculate_times();
    if (is_EBD_based) {
        A_est1 = std::max(0.0, A_est);
        A_est1 = std::max(0.0, std::min(0.4, A_est));
        double V_delta0 = Q_NVINHSMICPERM ? 0 : V_delta;
        double V_delta1 = A_est1*T_traction;
        double V_delta2 = A_est2*T_berem;
        double V_bec = std::max(V_est+V_delta0+V_delta1, V_target)+V_delta2;
        double D_bec = std::max(V_est+V_delta0+V_delta1/2, V_target)*T_traction + (std::max(V_est+V_delta0+V_delta1, V_target)+V_delta2/2)*T_berem;
        d_EBI = get_distance_curve(V_bec)-D_bec;
        d_SBI2 = d_EBI - V_est*T_bs2;
        d_W = d_SBI2 - V_est*T_warning;
        d_P = d_SBI2 - V_est*T_driver;
        double T_indication = std::max(0.8*T_bs, 5.0) + T_driver;
        d_I = d_P - T_indication*V_est;
        
        double D_be_display = (V_est+V_delta0+V_delta1/2)*T_traction + (V_est + V_delta0 + V_delta1 + V_delta2/2)*T_berem;
        distance v_sbi_dappr = d_maxsafefront(d_target.get_reference()) + V_est*T_bs2 + D_be_display;
        V_SBI2 = v_sbi_dappr < get_distance_curve(V_target) ? std::max(get_speed_curve(v_sbi_dappr)-(V_delta0+V_delta1+V_delta2),V_target + dV_sbi(V_target)) : (V_target + dV_sbi(V_target));
        
        //GUI disabled
        distance v_p_dappr = d_maxsafefront(d_target.get_reference()) + V_est*(T_driver+T_bs2) + D_be_display;
        V_P = v_p_dappr < get_distance_curve(V_target) ? std::max(get_speed_curve(v_p_dappr) - (V_delta0+V_delta1+V_delta2), V_target) : V_target;
    } else {
        d_SBI1 = get_distance_curve(V_est) - T_bs1*V_est;
        d_W = d_SBI1 - T_warning*V_est;
        d_P = d_SBI1 - T_driver*V_est;
        double T_indication = std::max(0.8*T_bs, 5.0) + T_driver;
        d_I = d_P - T_indication*V_est;
        
        distance v_sbi_dappr = d_estfront + V_est*T_bs1;
        V_SBI1 = v_sbi_dappr < d_target ? get_speed_curve(v_sbi_dappr) : 0;
            
        //GUI disabled
        distance v_p_dappr = d_estfront + V_est*(T_driver + T_bs1);
        V_P = v_p_dappr < d_target ? get_speed_curve(v_p_dappr) : 0;
    }
}
optional<distance> EoA;
optional<distance> SvL;
optional<distance> SR_dist;
optional<std::pair<distance,double>> LoA;
double V_releaseSvL=0;
static std::set<target> supervised_targets;
bool changed = false;
void set_supervised_targets()
{
    changed = true;
    supervised_targets.clear();
    std::map<distance, double> MRSP = get_MRSP();
    auto minMRSP = MRSP.begin();
    auto prev = minMRSP;
    for (auto it=++minMRSP; it!=MRSP.end(); ++it) {
        if (it->second < prev->second && d_maxsafefront(it->first.get_reference())<it->first)
            supervised_targets.insert(target(it->first, it->second, target_class::MRSP));
        prev = it;
    }
    if (SvL)
        supervised_targets.insert(target(*SvL, 0, target_class::SvL));
    if (EoA)
        supervised_targets.insert(target(*EoA, 0, target_class::EoA));
    if (SR_dist)
        supervised_targets.insert(target(*SR_dist, 0, target_class::SR_distance));
    if (LoA)
        supervised_targets.insert(target(LoA->first, LoA->second, target_class::LoA));
}
bool supervised_targets_changed()
{
    std::set<target> old;
    for (auto it = supervised_targets.begin(); it!=supervised_targets.end(); ++it) {
        if (it->type == target_class::MRSP && d_maxsafefront(it->get_target_position().get_reference()) >= it->get_target_position())
            old.insert(*it);
    }
    for (auto it = old.begin(); it!=old.end(); ++it)
        supervised_targets.erase(*it);
    if (changed || !old.empty()) {
        changed = false;
        return true;
    }
    return false;
}
std::set<target> get_supervised_targets()
{
    return supervised_targets;
}
void target::calculate_decelerations()
{
    calculate_decelerations(get_gradient());
}
void target::calculate_decelerations(std::map<distance,double> gradient)
{
    std::map<distance,int> redadh;
    redadh[distance(0)] = 0;
    acceleration A_gradient = get_A_gradient(gradient);
    acceleration A_brake_safe;
    if (conversion_model_used) {
        A_brake_safe = A_brake_emergency;
        A_brake_safe.accel = [=](double V, distance d) {
            return (--Kv_int.upper_bound(V))->second*(--Kr_int.upper_bound(L_TRAIN))->second*A_brake_emergency(V,d);
        };
        for (auto it=Kv_int.begin(); it!=Kv_int.end(); ++it)
            A_brake_safe.speed_step.insert(it->first);
    } else {
        A_brake_safe = A_brake_emergency;
        A_brake_safe.accel = [=](double V, distance d) {
            double wet = (--Kwet_rst.upper_bound(V))->second;
            return Kdry_rst(V,M_NVEBCL,d)*(wet+M_NVAVADH*(1-wet))*A_brake_emergency(V,d);
        };
    }
        
    A_safe = A_brake_safe + A_gradient;
    for (auto it=redadh.begin(); it!=redadh.end(); ++it)
        A_normal_service.dist_step.insert(it->first);
    A_safe.accel = [=](double V, distance d) {
        int adh = (--redadh.upper_bound(d))->second;
        double A_MAXREDADH = (adh == 3 ? A_NVMAXREDADH3 : (adh == 2 ? A_NVMAXREDADH2 : A_NVMAXREDADH1));
        return std::min(A_brake_safe(V,d), A_MAXREDADH) + A_gradient(V,d); 
    };
        
    A_expected = A_brake_service + A_gradient;
        
    A_normal_service = A_brake_normal_service + A_gradient;
    A_normal_service.accel = [=](double V, distance d) {
        double grad = (--get_gradient().upper_bound(d))->second;
        double kn = (grad > 0) ? (--Kn.upper_bound(V))->second.first : (--Kn.upper_bound(V))->second.second;
        return A_brake_normal_service(V,d) + A_gradient(V,d) - kn*grad/1000;
    };
    for (auto it=Kn.begin(); it!=Kn.end(); ++it)
        A_normal_service.speed_step.insert(it->first);
}