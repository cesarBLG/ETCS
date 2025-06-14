/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "targets.h"
#include "curve_calc.h"
#include "speed_profile.h"
#include "conversion_model.h"
#include "supervision.h"
#include "supervision_targets.h"
#include "national_values.h"
#include "fixed_values.h"
#include "train_data.h"
#include "track_pbd.h"
#include "../TrackConditions/track_condition.h"
#include "../MA/movement_authority.h"
#include "../TrainSubsystems/train_interface.h"
#include "../TrainSubsystems/power.h"
#include <set>
std::list<std::shared_ptr<PBD_target>> PBDs;
target::target(relocable_dist_base dist, double speed, target_class type, bool is_TSR) : basic_target(dist, speed, type, is_TSR)
{
    if (is_TSR)
        default_gradient = default_gradient_tsr;
    calculate_decelerations();
}
dist_base target::get_distance_curve(double velocity) const
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
double target::get_speed_curve(dist_base dist) const
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
dist_base target::get_distance_gui_curve(double velocity) const
{
    dist_base guifoot;
    if (type == target_class::EoA || type == target_class::SvL) {
        guifoot = d_target;
    } else {
        double V_delta0t = 0;
        dist_base debi = get_distance_curve(V_target+V_delta0t)-(V_target+V_delta0t)*(T_berem+T_traction);
        guifoot = debi-V_target*(T_driver+T_bs2);
    }
    return distance_curve(A_normal_service, guifoot, V_target, velocity);
}
double target::get_speed_gui_curve(dist_base dist) const
{
    dist_base guifoot;
    if (type == target_class::EoA || type == target_class::SvL) {
        guifoot = d_target;
    } else {
        double V_delta0t = 0.007*V_target;
        dist_base debi = get_distance_curve(V_target+V_delta0t)-(V_target+V_delta0t)*(T_berem+T_traction);
        guifoot = debi-V_target*(T_driver+T_bs2);
    }
    return speed_curve(A_normal_service, guifoot, V_target, dist);
}
void target::calculate_times() const
{
    if (conversion_model_used) {
        if (V_target > 0) {
            T_brake_emergency = T_brake_emergency_cmt;
            T_brake_service = T_brake_service_cmt;
        } else {
            T_brake_emergency = T_brake_emergency_cm0;
            T_brake_service = T_brake_service_cm0;
        }
    } else {
        T_brake_service = get_T_brake_service(d_estfront);
        T_brake_emergency = get_T_brake_emergency(d_estfront);
    }
    T_be = (conversion_model_used ? Kt_int : 1)*T_brake_emergency;
    T_bs = T_brake_service;
    if (Q_NVSBFBPERM) {
        T_bs1 = ::T_bs1;
        T_bs2 = ::T_bs2;
    } else {
        T_bs1 = T_bs2 = T_bs;
    }
    if (traction_cutoff_implemented)
        T_traction = std::max(0.0, T_traction_cutoff-(T_warning+T_bs2));
    else
        T_traction = T_traction_cutoff;
    T_berem = std::max(0.0, T_be-T_traction);
}
void target::calculate_curves(double V_est, double A_est, double V_delta) const
{
    calculate_times();
    if (is_EBD_based) {
        A_est1 = std::max(0.0, A_est);
        A_est2 = std::max(0.0, std::min(0.4, A_est));
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
        dist_base v_sbi_dappr = d_maxsafefront(d_target) + V_est*T_bs2 + D_be_display;
        V_SBI2 = v_sbi_dappr < get_distance_curve(V_target) ? std::max(get_speed_curve(v_sbi_dappr)-(V_delta0+V_delta1+V_delta2),V_target + dV_sbi(V_target)) : (V_target + dV_sbi(V_target));
        
        dist_base v_p_dappr = d_maxsafefront(d_target) + V_est*(T_driver+T_bs2) + D_be_display;
        if (v_p_dappr < get_distance_curve(V_target) || (Q_NVGUIPERM && d_maxsafefront(d_target) < get_distance_gui_curve(V_target))) {
            V_P = get_speed_curve(v_p_dappr) - (V_delta0+V_delta1+V_delta2);
            if (Q_NVGUIPERM)
                V_P = std::min(V_P, get_speed_gui_curve(d_maxsafefront(d_target)));
            V_P = std::max(V_P, V_target);
        } else {
            V_P = V_target;
        }
    } else {
        d_SBI1 = get_distance_curve(V_est) - T_bs1*V_est;
        d_W = d_SBI1 - T_warning*V_est;
        d_P = d_SBI1 - T_driver*V_est;
        double T_indication = std::max(0.8*T_bs, 5.0) + T_driver;
        d_I = d_P - T_indication*V_est;
        
        dist_base v_sbi_dappr = d_estfront_dir[d_target.orientation == -1] + V_est*T_bs1;
        V_SBI1 = v_sbi_dappr < d_target ? get_speed_curve(v_sbi_dappr) : 0;
            
        dist_base v_p_dappr = d_estfront_dir[d_target.orientation == -1] + V_est*(T_driver + T_bs1);
        if (v_p_dappr < d_target) {
            V_P = get_speed_curve(v_p_dappr);
            if (Q_NVGUIPERM)
                V_P = std::min(V_P, get_speed_gui_curve(d_estfront));
        } else {
            V_P = 0;
        }
    }
}
optional<distance> EoA;
optional<distance> SvL;
optional<distance> SR_dist_start;
optional<distance> SR_dist;
optional<double> SR_dist_override;
optional<double> SR_speed_override;
optional<std::pair<distance,double>> LoA;
double V_releaseSvL=0;
static std::list<std::shared_ptr<target>> supervised_targets;
bool changed = false;
void recalculate_all_decelerations();
void set_supervised_targets()
{
    update_brake_contributions();
    changed = true;
    indication_target = nullptr;
    supervised_targets.clear();
    if (mode != Mode::SR && mode != Mode::UN && mode != Mode::FS && mode != Mode::OS && mode != Mode::LS) return;
    auto &MRSP = get_MRSP();
    if (!MRSP.empty()) {
        auto minMRSP = MRSP.begin();
        auto prev = minMRSP;
        for (auto it=++minMRSP; it!=MRSP.end(); ++it) {
            if (it->second < prev->second && d_maxsafefront(it->first)<it->first) {
                bool is_TSR = false;
                for (auto &tsr : TSRs) {
                    if (it->first == tsr.restriction.get_start() && it->second == tsr.restriction.get_speed()) {
                        is_TSR = true;
                        break;
                    }
                }
                auto t = std::make_shared<target>(it->first, it->second, target_class::MRSP, is_TSR);
                supervised_targets.push_back(t);
            }
            prev = it;
        }
    }
    if (mode == Mode::FS || mode == Mode::OS || mode == Mode::LS) {
        if (SvL)
            supervised_targets.push_back(std::make_shared<target>(SvL->max, 0, target_class::SvL));
        if (EoA)
            supervised_targets.push_back(std::make_shared<target>(EoA->est, 0, target_class::EoA));
        if (LoA)
            supervised_targets.push_back(std::make_shared<target>(LoA->first.max, LoA->second, target_class::LoA));
    }
    SR_dist = {};
    if (mode == Mode::SR && SR_dist_start) {
        double D_STFF = D_NVSTFF;
        if (SR_dist_override)
            D_STFF = *SR_dist_override;
        if (std::isfinite(D_STFF))
            SR_dist = *SR_dist_start + D_STFF;
        if (SR_dist)
            supervised_targets.push_back(std::make_shared<target>(SR_dist->max, 0, target_class::SR_distance));
    }
    target::recalculate_all_decelerations();
}
bool supervised_targets_changed()
{
    bool removed = false;
    for (auto it = supervised_targets.begin(); it!=supervised_targets.end(); ) {
        if ((*it)->type == target_class::MRSP && d_maxsafefront((*it)->get_target_position()) >= (*it)->get_target_position()) {
            removed = true;
            it = supervised_targets.erase(it);
        } else {
            ++it;
        }
    }
    if (changed || removed) {
        changed = false;
        return true;
    }
    return false;
}
const std::list<std::shared_ptr<target>> &get_supervised_targets()
{
    return supervised_targets;
}
void target::calculate_decelerations()
{
    calculate_decelerations(get_gradient());
}
void target::calculate_decelerations(const std::map<dist_base,double> &gradient)
{
    std::map<dist_base,bool> redadh;
    redadh[dist_base(std::numeric_limits<double>::lowest(), 0)] = false;
    acceleration A_gradient = get_A_gradient(gradient, default_gradient);
    acceleration A_brake_emergency = get_A_brake_emergency(use_brake_combination);
    acceleration A_brake_service = get_A_brake_service(use_brake_combination);
    acceleration A_brake_normal_service = get_A_brake_normal_service(A_brake_service);
    acceleration A_brake_safe;
    if (conversion_model_used) {
        A_brake_safe = A_brake_emergency;
        for (auto it=Kv_int.begin(); it!=Kv_int.end(); ++it)
            A_brake_safe.speed_step.insert(it->first);

        for (auto &d : A_brake_safe.dist_step) {
            for (auto &V : A_brake_safe.speed_step) {
                A_brake_safe.accelerations[d][V] = (--Kv_int.upper_bound(V))->second*(--Kr_int.upper_bound(L_TRAIN))->second*A_brake_emergency(V,d);
            }
        }
    } else {
        A_brake_safe = A_brake_emergency;
        for (auto &d : A_brake_safe.dist_step) {
            for (auto &V : A_brake_safe.speed_step) {
                double wet = Kwet_rst(V,d);
                A_brake_safe.accelerations[d][V] = Kdry_rst(V,M_NVEBCL,d)*(wet+M_NVAVADH*(1-wet))*A_brake_emergency(V,d);
            }
        }
    }

    A_safe = A_brake_safe + A_gradient;
    for (auto it=redadh.begin(); it!=redadh.end(); ++it)
        A_safe.dist_step.insert(it->first);
    for (auto &d : A_safe.dist_step) {
        for (auto &V : A_safe.speed_step) {
            bool slip = (--redadh.upper_bound(d))->second || slippery_rail_driver;
            double A_MAXREDADH = slip ? (brake_position != brake_position_types::PassengerP ? A_NVMAXREDADH3 : (additional_brake_available ? A_NVMAXREDADH2 : A_NVMAXREDADH1)) : -3;
            if (!slip || A_MAXREDADH < 0)
                A_MAXREDADH = std::numeric_limits<double>::max();
            A_safe.accelerations[d][V] =  std::min(A_brake_safe(V,d), A_MAXREDADH) + A_gradient(V,d);
        }
    }
        
    A_expected = A_brake_service + A_gradient;
    
    A_normal_service = A_brake_normal_service + A_gradient;
    if (!Kn[0].empty() && !Kn[1].empty()) {
        for (auto it=Kn[0].begin(); it!=Kn[0].end(); ++it)
            A_normal_service.speed_step.insert(it->first);
        for (auto it=Kn[1].begin(); it!=Kn[1].end(); ++it)
            A_normal_service.speed_step.insert(it->first);
        for (auto &d : A_normal_service.dist_step) {
            for (auto &V : A_normal_service.speed_step) {
                double grad = (gradient.empty() || gradient.begin()->first > d) ? default_gradient : (--gradient.upper_bound(d))->second;
                double kn = (grad > 0) ? (--Kn[0].upper_bound(V))->second : (--Kn[1].upper_bound(V))->second;
                A_normal_service.accelerations[d][V] = A_brake_normal_service(V,d) + A_gradient(V,d) - kn*grad/1000;
            }
        }
    }
}
void target::recalculate_all_decelerations()
{
    std::vector<std::shared_ptr<target>> targets; 
    for (auto &tc : track_conditions) {
        for (auto &t : tc->targets) {
            t->calculate_decelerations();
        }
    }
    calculate_perturbation_location();
}
void PBD_target::calculate_restriction()
{
    calculate_times();
    float V_PBD = 0;
    dist_base doffset = (is_EBD_based ? start.max : start.est) + L_antenna_front;
    for (double v_pbd = 0; v_pbd<500/3.6; v_pbd+=0.8/3.6) {
        if (is_EBD_based) {
            float V_delta0PBD = Q_NVINHSMICPERM ? 0 : 0;
            float Dbec = (v_pbd + dV_ebi(v_pbd) + V_delta0PBD)*(T_traction + T_berem);
            dist_base d1 = doffset + Dbec;
            if (d1 <= d_target && abs(v_pbd+dV_ebi(v_pbd)-(speed_curve(A_safe, d_target, 0, d1)-V_delta0PBD))<=1/3.6) {
                V_PBD = v_pbd;
                break;
            }
        } else {
            float V_delta0PBD = Q_NVINHSMICPERM ? 0 : 0;
            float Dbec = (v_pbd + dV_sbi(v_pbd) + V_delta0PBD)*(T_traction + T_berem);
            dist_base d1 = doffset + Dbec + (v_pbd + dV_sbi(v_pbd))*T_bs2;
            if (d1 <= d_target && abs(v_pbd+dV_sbi(v_pbd)-(speed_curve(A_safe, d_target, 0, d1)-V_delta0PBD))<=1/3.6) {
                V_PBD = v_pbd;
                break;
            }
        }
    }
    if (!is_EBD_based) {
        float V_PBD_SB = 0;
        for (double v_pbd = 0; v_pbd<500/3.6; v_pbd+=0.8/3.6) {
            dist_base d1 = doffset + (v_pbd + dV_sbi(v_pbd))*T_bs1;
            if (d1 <= d_target && abs(v_pbd+dV_sbi(v_pbd)-(speed_curve(A_expected, d_target, 0, d1)))<=1/3.6) {
                V_PBD_SB = v_pbd;
                break;
            }
        }
        if (V_PBD > V_PBD_SB)
            V_PBD = V_PBD_SB;
    }
    restriction = speed_restriction(((int)(V_PBD*3.6/5))*5/3.6, start, end, false);
}
void load_PBD(PermittedBrakingDistanceInformation &pbd, distance ref)
{
    if (pbd.Q_TRACKINIT == pbd.Q_TRACKINIT.InitialState) {
        distance resume = ref + pbd.D_TRACKINIT.get_value(pbd.Q_SCALE);
        delete_PBD(resume);
    }
    distance start = ref+pbd.element.D_PBDSR.get_value(pbd.Q_SCALE);
    delete_PBD(start);
    std::vector<PBD_element> elements;
    elements.push_back(pbd.element);
    elements.insert(elements.end(), pbd.elements.begin(), pbd.elements.end());
    for (auto &e : elements) {
        ref += e.D_PBDSR.get_value(pbd.Q_SCALE);
        double grad = (e.Q_GDIR == e.Q_GDIR.Uphill ? 0.001 : -0.001)*e.G_PBDSR;
        PBDs.push_back(std::make_shared<PBD_target>(ref, ref+e.L_PBDSR.get_value(pbd.Q_SCALE), e.D_PBD.get_value(pbd.Q_SCALE), e.Q_PBDSR == e.Q_PBDSR.EBIntervention, grad));
    }
}