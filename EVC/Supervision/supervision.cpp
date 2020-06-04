/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <map>
#include "national_values.h"
#include "fixed_values.h"
#include "../Position/distance.h"
#include "../Position/linking.h"
#include "../MA/movement_authority.h"
#include "train_data.h"
#include "speed_profile.h"
#include "targets.h"
#include "supervision.h"
#include "../antenna.h"
#include "../TrainSubsystems/brake.h"
#include <iostream>
#include <cmath>
double V_est=0;
double V_ura = 0;
double A_est = 0;
double V_perm;
double V_target;
double V_sbi;
double D_target;
double TTI;
double TTP;
bool EB=false;
bool SB=false;
bool TCO=false;
MonitoringStatus monitoring = CSM;
SupervisionStatus supervision = NoS;
target MRDT;
target RSMtarget;
distance d_startRSM;
target indication_target;
double indication_distance;
double V_release = 0;
double T_brake_service;
double T_brake_emergency;
double T_bs;
double T_bs1;
double T_bs2;
double T_be;
double calc_ceiling_limit()
{
    std::map<distance,double> MRSP = get_MRSP();
    double V_MRSP = 1000;
    for (auto it = MRSP.begin(); it!=MRSP.end(); ++it) {
        distance d = it->first;
        distance min = d_minsafefront(d.get_reference());
        distance max = d_maxsafefront(d.get_reference());
        auto next = it;
        ++next;
        if ((max>d && min<d) || (min>d && (next==MRSP.end() || min<next->first)))
            V_MRSP = std::min(it->second, V_MRSP);
    }
    return V_MRSP;
}
double calc_ceiling_limit(distance min, distance max)
{
    std::map<distance,double> MRSP = get_MRSP();
    auto it1 = --MRSP.upper_bound(min);
    auto it2 = MRSP.upper_bound(max);
    double V_MRSP = 1000;
    for (auto it = it1; it!=it2; ++it) {
        V_MRSP = std::min(it->second, V_MRSP);
    }
    return V_MRSP;
}
distance get_d_startRSM(double V_release)
{
    distance d_SvL = *SvL;
    distance d_EoA = *EoA;
    const std::list<target> &supervised_targets = get_supervised_targets();
    const target *tEoA, *tSvL;
    for (auto it = supervised_targets.begin(); it != supervised_targets.end(); ++it) {
        if (it->type == target_class::EoA)
            tEoA = &*it;
        if (it->type == target_class::SvL)
            tSvL = &*it;
    }
    int alpha = level==Level::N1;
    distance d_tripEoA = d_EoA+alpha*L_antenna_front + std::max(2*Q_LOCACC_LRBG+10+d_EoA.get()/10,d_maxsafefront(d_EoA.get_reference())-d_minsafefront(d_EoA.get_reference()));
    
    distance d_startRSM;
    
    tEoA->calculate_times();
    distance d_sbi1 = tEoA->get_distance_curve(V_release)-V_release*tEoA->T_bs1;
    distance d_sbi2(d_SvL);
    std::list<target> candidates;
    if (V_releaseSvL == -2) {
        for (auto it=supervised_targets.begin(); it!=supervised_targets.end(); ++it) {
            if (it->is_EBD_based && d_tripEoA < it->get_target_position() && it->get_target_position() <= d_SvL)
                candidates.push_back(*it);
        }
    }
    candidates.push_back(*tSvL);
    for (auto &t : candidates) {
        t.calculate_times();
        double V_delta0rs = 0.007*V_release;
        distance d_ebit = t.get_distance_curve(V_release + V_delta0rs)-(V_release*V_delta0rs)*(t.T_berem+t.T_traction);
        distance d_sbi2t = d_ebit - V_release * t.T_bs2;
        if (d_sbi2t<d_sbi2) {
            d_sbi2 = d_sbi2t;
            RSMtarget = t;
        }
    }
    if (d_sbi2-d_sbi1>=d_maxsafefront(d_EoA.get_reference())-d_estfront) {
        d_startRSM = d_sbi1;
        RSMtarget = *tEoA;
    } else {
        d_startRSM = d_sbi2;
    }
    return d_startRSM;
}
double calculate_V_release()
{
    if (V_releaseSvL == -1)
        return V_NVREL;
    else if (V_releaseSvL >= 0)
        return V_releaseSvL;
    distance d_SvL = *SvL;
    distance d_EoA = *EoA;
    std::list<target> supervised_targets = get_supervised_targets();
    int alpha = level==Level::N1;
    distance d_tripEoA = d_EoA+alpha*L_antenna_front + std::max(2*Q_LOCACC_LRBG+10+d_EoA.get()/10,d_maxsafefront(d_EoA.get_reference())-d_minsafefront(d_EoA.get_reference()));
    double V_release = calc_ceiling_limit(d_EoA, d_SvL);
    std::list<target*> candidates;
    target *tSvL;
    for (auto it=supervised_targets.begin(); it!=supervised_targets.end(); ++it) {
        if (it->is_EBD_based && d_tripEoA < it->get_target_position() && it->get_target_position() <= d_SvL)
            candidates.push_back(&*it);
        if (it->type == target_class::SvL)
            tSvL = &*it;
    }
    candidates.push_back(tSvL);
    for (target *t : candidates) {
        t->calculate_times();
        double V_target = t->get_target_speed();
        double V_releaset = V_target;
        double V_test = V_target;
        while (V_test <= V_release) {
            double V_delta0rsob = Q_NVINHSMICPERM ? 0 : std::max(0.007*V_release, V_ura);
            double D_bec = (V_test+V_delta0rsob)*(t->T_traction+t->T_berem);
            if (d_tripEoA+D_bec<=t->get_distance_curve(V_target) && std::abs(V_test-(t->get_speed_curve(d_tripEoA+D_bec)-V_delta0rsob))<=(1.0/3.6)) {
                V_releaset = V_test;
                break;
            }
            V_test += 1.0/3.6;
        }
        V_release = std::min(V_release, V_releaset);
    }
    if (V_release == 0)
        return 0;
    distance d_start = get_d_startRSM(V_release);
    double V_MRSP = calc_ceiling_limit(d_start, d_tripEoA);
    return std::min(V_release, V_MRSP);
}
void update_monitor_transitions(bool suptargchang, const std::list<target> &supervised_targets)
{
    MonitoringStatus nmonitor = monitoring;
    bool c1 = false;
    bool mrdt = false;
    for (const target &t : supervised_targets) {
        c1 |= (t.get_target_speed()<=V_est) && ((t.is_EBD_based ? d_maxsafefront(t.d_I.get_reference()) : d_estfront) > t.d_I);
        if (MRDT == t)
            mrdt = true;
    }
    c1 &= (V_est>=V_release);
    bool c2 = V_release>0 && (RSMtarget.is_EBD_based ? d_maxsafefront(d_startRSM.get_reference()) : d_estfront) > d_startRSM;
    bool c3 = !c1 && !c2 && !mrdt;
    bool c4 = c1 && suptargchang;
    bool c5 = c2 && suptargchang;
    if (c1 && monitoring == CSM) {
        nmonitor = TSM;
    }
    if (c2 && monitoring != RSM)
        nmonitor = RSM;
    if (c3 && monitoring != CSM)
        nmonitor = CSM;
    if (c4 && monitoring != TSM)
        nmonitor = TSM;
    if (c5 && monitoring != RSM)
        nmonitor = RSM;
    if (monitoring!=nmonitor) {
        if (monitoring == TSM)
            TCO = false;
        if (monitoring == TSM && nmonitor == RSM)
            SB = false;
        monitoring = nmonitor;
    }
}
#include <chrono>
#include <iostream>
optional<distance> standstill_position;
bool standstill_applied;
bool standstill_acknowledged;
optional<distance> rollaway_position;
bool rollaway_applied;
bool rollaway_acknowledged;
void update_supervision()
{
    if (mode == Mode::TR) {
        EB = true;
        V_perm = V_target = V_release = V_sbi = 0;
        supervision = IntS;
        return;
    }
    if (mode == Mode::SB) {
        if (!standstill_position)
            standstill_position = d_estfront;
        if (std::abs(d_estfront - *standstill_position) > D_NVROLL && !standstill_applied) {
            trigger_brake_reason(1);
            standstill_applied = true;
            standstill_acknowledged = false;
        }
        if (standstill_acknowledged) {
            standstill_position = d_estfront;
            standstill_applied = standstill_acknowledged = false;
        }
    } else {
        standstill_position = {};
        standstill_applied = standstill_acknowledged = false;
    }
    if (mode == Mode::FS || mode == Mode::OS || mode == Mode::SR || mode == Mode::LS || mode == Mode::PT || mode == Mode::RV || mode == Mode::SH || mode == Mode::UN) {
    } else {
        rollaway_position = {};
        rollaway_applied = rollaway_acknowledged = false;
    }
    if (mode == Mode::FS || mode == Mode::OS || mode == Mode::SR || mode == Mode::LS || mode == Mode::PT || mode == Mode::RV) {
        // Reverse movement protection
    } else {

    }
    if (!(mode == Mode::OS || mode == Mode::FS || mode == Mode::LS || mode == Mode::SN ||
        mode == Mode::SR || mode == Mode::SH || mode == Mode::UN || mode == Mode::RV))
        return;

    if (MA)
        MA->update_timers();
    
    bool suptargchang = supervised_targets_changed();
    
    double V_MRSP = calc_ceiling_limit();
    if (LoA && d_maxsafefront(LoA->first.get_reference())>LoA->first)
        V_MRSP = std::min(LoA->second, V_MRSP);
    double prev_V_perm = V_perm;
    double prev_D_target = D_target;
    double prev_V_sbi = V_sbi;
    V_perm = V_target = V_MRSP;
    V_sbi = V_MRSP + dV_sbi(V_MRSP);
    
    target tEoA;
    target tSvL;
    const std::list<target> &supervised_targets = get_supervised_targets();
    for (auto it=supervised_targets.begin(); it!=supervised_targets.end(); ++it) {
        it->calculate_curves();
        if (it->type == target_class::SvL)
            tSvL = *it;
        if (it->type == target_class::EoA)
            tEoA = *it;
    }
    if (EoA && SvL) {
        if (V_release != 0)
            d_startRSM = get_d_startRSM(V_release);
    } else {
        V_release = 0;
    }
    update_monitor_transitions(suptargchang, supervised_targets);
    if (monitoring == CSM) {
        bool t1 = V_est <= V_MRSP;
        bool t2 = V_est > V_MRSP;
        bool t3 = V_est > V_MRSP + dV_warning(V_MRSP);
        bool t4 = V_est > V_MRSP + dV_sbi(V_MRSP);
        bool t5 = V_est > V_MRSP + dV_ebi(V_MRSP);
        bool r0 = V_est == 0;
        bool r1 = V_est <= V_MRSP;
        if (t1 && false) { //TODO: Speed and distance monitoring first entered
            supervision = NoS;
        }
        if (t2 && (supervision == NoS || supervision == IndS))
            supervision = OvS;
        if (t3 && (supervision == NoS || supervision == IndS || supervision == OvS))
            supervision = WaS;
        if ((t4 || t5) && (supervision == NoS || supervision == IndS || supervision == OvS || supervision == WaS))
            supervision = IntS;
        if (r1 && (supervision == IndS || supervision == OvS || supervision == WaS))
            supervision = NoS;
        if ((r0 || (r1 && (!EB || Q_NVEMRRLS))) && supervision == IntS)
            supervision = NoS;
        if (t4)
            SB = true;
        if (t5)
            EB = true;
        if (r0)
            EB = false;
        if (r1) {
            SB = false;
            if (Q_NVEMRRLS)
                EB = false;
        }
        if (V_est < V_release) {
            indication_target = RSMtarget;
            if (RSMtarget.is_EBD_based)
                indication_distance = d_startRSM-d_maxsafefront(d_startRSM.get_reference());
            else
                indication_distance = d_startRSM-d_estfront;
        } else {
            bool asig=false;
            for (target t : supervised_targets) {
                double d = t.d_I - (t.is_EBD_based ? d_maxsafefront(t.d_I.get_reference()) : d_estfront);
                if (!asig || indication_distance>d) {
                    indication_distance = d;
                    indication_target = t;
                    asig = true;
                }
            }
        }
    } else if (monitoring == TSM) {
        std::list<const target*> MRDTtarg;
        for (const target &t : supervised_targets) {
            if ((t.type == target_class::EoA ? d_estfront : d_maxsafefront(t.d_I.get_reference())) > t.d_I && V_est>=t.get_target_speed())
                MRDTtarg.push_back(&t);
        }
        if (!MRDTtarg.empty())
        {
            std::vector<const target*> MRDT;
            MRDT.push_back(*MRDTtarg.begin());
            for (const target *t : MRDTtarg) {
                if (t->V_P < MRDT[0]->V_P)
                    MRDT[0] = t;
            }
            for (int i=1; i<MRDTtarg.size(); i++) {
                bool mask = false;
                for (const target *t : MRDTtarg) {
                    bool already = false;
                    for (int j=0; j<i; j++) {
                        if (MRDT[j]==t) {
                            already = true;
                            break;
                        }
                    }
                    if (already)
                        continue;
                    t->calculate_curves(MRDT[i-1]->get_target_speed());
                    if (t->d_I < MRDT[i-1]->d_P) {
                        mask = true;
                        MRDT.push_back(t);
                        break;
                    }
                }
                if (!mask)
                    break;
            }
            const target &newMRDT = *MRDT[MRDT.size()-1];
            if (::MRDT.type != newMRDT.type || ::MRDT.get_target_speed() != newMRDT.get_target_speed())
                send_command("playSinfo","");
            ::MRDT = newMRDT;
        }
        TTP = 20;
        bool t3=false;
        bool t4=false;
        bool t6=false;
        bool t7=false;
        bool t9 = false;
        bool t10=false;
        bool t12=false;
        bool t13=false;
        bool t15=false;
        bool r0 = true;
        bool r1 = true;
        bool r3 = true;
        for (const target &t : supervised_targets) {
            double V_target = t.get_target_speed();
            distance &d_EBI = t.d_EBI;
            distance &d_SBI2 = t.d_SBI2;
            distance &d_SBI1 = t.d_SBI1;
            distance &d_W = t.d_W;
            distance &d_P = t.d_P;
            distance &d_I = t.d_I;
            if (t.type == target_class::MRSP || t.type == target_class::LoA) {
                distance d_maxsafe = d_maxsafefront(t.get_target_position().get_reference());
                t3 |= V_target<V_est && V_est<=V_MRSP && d_I<d_maxsafe && d_maxsafe <=d_P;
                t4 |= V_target<V_est && V_est<=V_MRSP && d_maxsafe > d_P;
                t6 |= V_MRSP<V_est && V_est<=V_MRSP+dV_warning(V_MRSP) && d_I<d_maxsafe && d_maxsafe<=d_W;
                t7 |= V_target + dV_warning(V_target) < V_est && V_est <= V_MRSP + dV_warning(V_MRSP) && d_maxsafe > d_W;
                t9 |= V_MRSP+dV_warning(V_MRSP)<V_est && V_est <= V_MRSP + dV_warning(V_MRSP) && d_I<d_maxsafe && d_maxsafe <= d_SBI2;
                t10 |= V_target + dV_sbi(V_target) < V_est && V_est <= V_MRSP + dV_sbi(V_MRSP) && d_maxsafe > d_SBI2;
                t12 |= V_MRSP + dV_sbi(V_MRSP) < V_est && V_est <= V_MRSP + dV_ebi(V_MRSP) && d_I<d_maxsafe && d_maxsafe <= d_EBI;
                t13 |= V_target + dV_ebi(V_target) < V_est && V_est <= V_MRSP + dV_ebi(V_MRSP) && d_maxsafe > d_EBI;
                t15 |= V_est > V_MRSP + dV_ebi(V_MRSP) && d_maxsafe > d_I;
                r0 &= V_est == 0;
                r1 &= V_est<=V_target;
                r3 &= V_target<V_est && V_est<=V_MRSP && d_maxsafe<=d_P;
                V_sbi = std::min(V_sbi, t.V_SBI2);
            } else if (t.type == target_class::EoA) {
                V_sbi = std::min(V_sbi, std::max(t.V_SBI1, V_release));
            } else if (t.type == target_class::SvL) {
                V_sbi = std::min(V_sbi, std::max(t.V_SBI2, V_release));
            } else if (t.type == target_class::SR_distance) {
                distance d_maxsafe = d_maxsafefront(t.get_target_position().get_reference());
                t3 |= 0<V_est && V_est<=V_MRSP && d_maxsafe>t.d_I && d_maxsafe<=t.d_P;
                t4 |= 0<V_est && V_est<=V_MRSP && d_maxsafe>t.d_P;
                t6 |= V_MRSP<V_est && V_est <= V_MRSP + dV_warning(V_MRSP) && d_maxsafe>t.d_I && d_maxsafe<=t.d_W;
                t7 |= 0<V_est && V_est <= V_MRSP + dV_warning(V_MRSP) && d_maxsafe>t.d_W;
                t9 |= V_MRSP + dV_warning(V_MRSP) < V_est && V_est <= V_MRSP + dV_sbi(V_MRSP) && d_maxsafe>t.d_I && d_maxsafe<=t.d_SBI2;
                t10 |= 0<V_est && V_est <= V_MRSP + dV_sbi(V_MRSP) && d_maxsafe>t.d_SBI2;
                t12 |= V_MRSP + dV_sbi(V_MRSP) < V_est && V_est <= V_MRSP + dV_ebi(V_MRSP) && d_maxsafe>t.d_I && d_maxsafe<=t.d_EBI;
                t13 |= 0<V_est && V_est <= V_MRSP + dV_ebi(V_MRSP) && d_maxsafe>t.d_EBI;
                t15 |= V_est > V_MRSP + dV_ebi(V_MRSP) && d_maxsafe>t.d_I;
                r0 &= V_est == 0;
                r1 &= V_est<=0;
                r3 &= 0<V_est && V_est<=V_MRSP && d_maxsafe<t.d_P;
            }
            V_perm = std::min(V_perm, t.V_P);
            TTP = std::min(TTP, (d_P - (t.is_EBD_based ? d_maxsafefront(d_P.get_reference()) : d_estfront))/V_est);
        }
        MRDT.calculate_curves(MRDT.get_target_speed());
        V_target = MRDT.get_target_speed();
        if (MRDT.type == target_class::EoA || MRDT.type == target_class::SvL)
            D_target = std::max(std::min(*EoA-d_estfront, *SvL-d_maxsafefront(SvL->get_reference())), 0.0);
        else
            D_target = std::max(MRDT.d_P-d_maxsafefront(MRDT.get_target_position().get_reference()), 0.0);
        
        if (EoA && SvL) {
            distance d_maxsafe = d_maxsafefront(EoA->get_reference());
            t3 |= V_release<V_est && V_est<=V_MRSP && (d_maxsafe>tSvL.d_I || d_estfront>tEoA.d_I) && (d_maxsafe<=tSvL.d_P && d_estfront<=tEoA.d_P);
            t4 |= V_release<V_est && V_est<=V_MRSP && (d_maxsafe>tSvL.d_P || d_estfront > tEoA.d_P);
            t6 |= V_MRSP<V_est && V_est <= V_MRSP + dV_warning(V_MRSP) && (d_maxsafe>tSvL.d_I || d_estfront>tEoA.d_I) && (d_maxsafe<=tSvL.d_W && d_estfront <= tEoA.d_W);
            t7 |= V_release<V_est && V_est <= V_MRSP + dV_warning(V_MRSP) && (d_maxsafe>tSvL.d_W || d_estfront>tEoA.d_W);
            t9 |= V_MRSP + dV_warning(V_MRSP) < V_est && V_est <= V_MRSP + dV_sbi(V_MRSP) && (d_maxsafe>tSvL.d_I || d_estfront>tEoA.d_I) && (d_maxsafe<=tSvL.d_SBI2 && d_estfront <= tEoA.d_SBI1);
            t10 |= V_release<V_est && V_est <= V_MRSP + dV_sbi(V_MRSP) && (d_maxsafe>tSvL.d_SBI2 || d_estfront>tEoA.d_SBI1);
            t12 |= V_MRSP + dV_sbi(V_MRSP) < V_est && V_est <= V_MRSP + dV_ebi(V_MRSP) && (d_maxsafe>tSvL.d_I || d_estfront>tEoA.d_I) && d_maxsafe<=tSvL.d_EBI;
            t13 |= V_release<V_est && V_est <= V_MRSP + dV_ebi(V_MRSP) && d_maxsafe>tSvL.d_EBI;
            t15 |= V_est > V_MRSP + dV_ebi(V_MRSP) && (d_maxsafe>tSvL.d_I || d_estfront>tEoA.d_I);
            r0 &= V_est == 0;
            r1 &= V_est<=V_release;
            r3 &= V_release<V_est && V_est<=V_MRSP && d_estfront<tEoA.d_P && d_maxsafe<tSvL.d_P;
        }
        if (t3 && supervision == NoS)
            supervision = IndS;
        if ((t4 || t6) && (supervision == NoS || supervision == IndS))
            supervision = OvS;
        if ((t7 || t9) && (supervision == NoS || supervision == IndS || supervision == OvS))
            supervision = WaS;
        if ((t10 || t12 || t13 || t15) && (supervision == NoS || supervision == IndS || supervision == OvS || supervision == WaS))
            supervision = IntS;
        if ((r1 || r3) && (supervision == WaS || supervision == OvS))
            supervision = IndS;
        if ((r0 || ((r1 || r3) && (!EB || Q_NVEMRRLS))) && supervision == IntS)
            supervision = IndS;
        if (t7 || t9)
            TCO = true;
        if (t10 || t12)
            SB = true;
        if (t13 || t15)
            EB= true;
        if (r0)
            EB = false;
        if (r1) {
            TCO = false;
            SB = false;
            if (Q_NVEMRRLS)
                EB = false;
        }
        if (r3) {
            TCO = false;
            SB = false;
            if (Q_NVEMRRLS)
                EB = false;
        }
    } else if (monitoring == RSM) {
        for (const target &t : supervised_targets) {
            V_perm = std::min(V_perm, t.V_P);
            D_target = std::max(std::min(*EoA-d_estfront, *SvL-d_maxsafefront(SvL->get_reference())), 0.0);
        }
        V_target = 0;
        V_sbi = std::min(V_sbi, V_release);
        bool t1 = V_est <= V_release;
        bool t2 = V_est > V_release;
        bool r0 = V_est == 0;
        bool r1 = V_est <= V_release;
        if (t1 && supervision == NoS)
            supervision = IndS;
        if (t2 && (supervision == NoS || supervision == IndS || supervision == OvS || supervision == WaS))
            supervision = IntS;
        if ((r1 && (supervision == OvS || supervision == WaS)) || (r0 && supervision == IntS))
            supervision = IndS;
        if (t2)
            EB = true;
        if (r0)
            EB = false;
    }
}
