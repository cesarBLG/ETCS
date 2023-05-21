/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "movement_authority.h"
#include "../Supervision/supervision.h"
#include "../LX/level_crossing.h"
#include "../Procedures/stored_information.h"
#include "../Supervision/emergency_stop.h"
#include "../TrackConditions/route_suitability.h"
optional<distance> d_perturbation_eoa;
optional<distance> d_perturbation_svl;
movement_authority::movement_authority(distance start, Level1_MA ma, int64_t time) : start(start), time_stamp(time)
{
    v_main = ma.V_MAIN.get_value();
    v_ema = ma.V_EMA.get_value();
    distance cumdist = start;
    for (int i=0; i<ma.N_ITER; i++) {
        ma_section section;
        section.length = ma.sections[i].L_SECTION.get_value(ma.Q_SCALE);
        if (ma.sections[i].Q_SECTIONTIMER && ma.sections[i].T_SECTIONTIMER != T_SECTIONTIMER_t::Infinity)
            section.stimer = section_timer(ma.sections[i].T_SECTIONTIMER, cumdist + ma.sections[i].D_SECTIONTIMERSTOPLOC.get_value(ma.Q_SCALE));
        sections.push_back(section);
        cumdist += section.length;
    }
    ma_section endsection;
    endsection.length = ma.L_ENDSECTION.get_value(ma.Q_SCALE);
    if (ma.Q_SECTIONTIMER && ma.T_SECTIONTIMER != T_SECTIONTIMER_t::Infinity)
        endsection.stimer = section_timer(ma.T_SECTIONTIMER, cumdist + ma.D_SECTIONTIMERSTOPLOC.get_value(ma.Q_SCALE));
    sections.push_back(endsection);
    cumdist += endsection.length;
    if (ma.Q_ENDTIMER && ma.T_ENDTIMER != T_SECTIONTIMER_t::Infinity)
        endtimer = end_timer(ma.T_ENDTIMER, cumdist-ma.D_ENDTIMERSTARTLOC.get_value(ma.Q_SCALE));
    if (ma.Q_OVERLAP) {
        overlap ov;
        ov.distance = ma.D_OL.get_value(ma.Q_SCALE);
        if (ma.V_RELEASEOL == V_release_t::CalculateOnBoard)
            ov.vrelease = -2;
        else if (ma.V_RELEASEOL == V_release_t::UseNationalValue)
            ov.vrelease = -1;
        else
            ov.vrelease = ma.V_RELEASEOL.get_value();
        if (ma.T_OL != T_OL_t::Infinity)
            ov.ovtimer = end_timer(ma.T_OL, cumdist-ma.D_STARTOL.get_value(ma.Q_SCALE));
        ol = ov;
    }
    if (ma.Q_DANGERPOINT) {
        danger_point d;
        d.distance = ma.D_DP.get_value(ma.Q_SCALE);
        if (ma.V_RELEASEDP == V_release_t::CalculateOnBoard)
            d.vrelease = -2;
        else if (ma.V_RELEASEDP == V_release_t::UseNationalValue)
            d.vrelease = -1;
        else
            d.vrelease = ma.V_RELEASEDP.get_value();
        dp = d;
    }
}
movement_authority::movement_authority(distance start, Level2_3_MA ma, int64_t time) : start(start), time_stamp(time)
{
    v_main = std::numeric_limits<double>::max();
    v_ema = ma.V_EMA.get_value();
    distance cumdist = start;
    for (int i=0; i<ma.N_ITER; i++) {
        ma_section section;
        section.length = ma.sections[i].L_SECTION.get_value(ma.Q_SCALE);
        if (ma.sections[i].Q_SECTIONTIMER && ma.sections[i].T_SECTIONTIMER != T_SECTIONTIMER_t::Infinity)
            section.stimer = section_timer(ma.sections[i].T_SECTIONTIMER, cumdist + ma.sections[i].D_SECTIONTIMERSTOPLOC.get_value(ma.Q_SCALE));
        sections.push_back(section);
        cumdist += section.length;
    }
    ma_section endsection;
    endsection.length = ma.L_ENDSECTION.get_value(ma.Q_SCALE);
    if (ma.Q_SECTIONTIMER && ma.T_SECTIONTIMER != T_SECTIONTIMER_t::Infinity)
        endsection.stimer = section_timer(ma.T_SECTIONTIMER, cumdist + ma.D_SECTIONTIMERSTOPLOC.get_value(ma.Q_SCALE));
    sections.push_back(endsection);
    cumdist += endsection.length;
    if (ma.Q_ENDTIMER && ma.T_ENDTIMER != T_SECTIONTIMER_t::Infinity)
        endtimer = end_timer(ma.T_ENDTIMER, cumdist-ma.D_ENDTIMERSTARTLOC.get_value(ma.Q_SCALE));
    if (ma.Q_OVERLAP) {
        overlap ov;
        ov.distance = ma.D_OL.get_value(ma.Q_SCALE);
        if (ma.V_RELEASEOL == V_release_t::CalculateOnBoard)
            ov.vrelease = -2;
        else if (ma.V_RELEASEOL == V_release_t::UseNationalValue)
            ov.vrelease = -1;
        else
            ov.vrelease = ma.V_RELEASEOL.get_value();
        if (ma.T_OL != T_OL_t::Infinity)
            ov.ovtimer = end_timer(ma.T_OL, cumdist-ma.D_STARTOL.get_value(ma.Q_SCALE));
        ol = ov;
    }
    if (ma.Q_DANGERPOINT) {
        danger_point d;
        d.distance = ma.D_DP.get_value(ma.Q_SCALE);
        if (ma.V_RELEASEDP == V_release_t::CalculateOnBoard)
            d.vrelease = -2;
        else if (ma.V_RELEASEDP == V_release_t::UseNationalValue)
            d.vrelease = -1;
        else
            d.vrelease = ma.V_RELEASEDP.get_value();
        dp = d;
    }
}
void set_data();
void movement_authority::calculate_distances()
{
	distance end = MA->get_end();
	if (MA->v_ema == 0) {
		LoA_ma = {};
		EoA_ma = distance(end);
		if (MA->ol) {
			SvL_ma = distance(end + MA->ol->distance);
			V_releaseSvL_ma = MA->ol->vrelease;
		} else if (MA->dp) {
			SvL_ma = distance(end + MA->dp->distance);
			V_releaseSvL_ma = MA->dp->vrelease;
		} else {
			SvL_ma = distance(end);
			V_releaseSvL_ma = 0;
		}
	} else {
		EoA_ma = SvL_ma = {};
		LoA_ma = {end, MA->v_ema};
	}
}
void movement_authority::update_timers()
{
	if (endtimer) {
		if (!endtimer->started && endtimer->startloc < d_maxsafefront(endtimer->startloc))
			endtimer->start();
		if (endtimer->triggered()) {
            endtimer = {};
            train_shorten('e');
		}
	}
	if (ol && ol->ovtimer) {
		if (!ol->ovtimer->started &&
			ol->ovtimer->startloc < d_maxsafefront(ol->ovtimer->startloc))
			ol->ovtimer->start();
		if (ol->ovtimer->triggered() || (ol->ovtimer->started && V_est <= 0)) {
			v_ema = 0;
			ol = {};
			set_data();
            svl_shorten('d');
		}
	}
	distance cumdist = start;
	for (int i = 0; i < sections.size(); i++) {
		if (sections[i].stimer) {
			if (!sections[i].stimer->started &&
				sections[i].stimer->stoploc > d_minsafefront(sections[i].stimer->stoploc)) {
				sections[i].stimer->start(time_stamp);
			}
			if (sections[i].stimer->stoploc <= d_minsafefront(sections[i].stimer->stoploc)) {
				sections[i].stimer->started = false;
			}
			if (sections[i].stimer->triggered()) {
                v_ema = 0;
				EoA_ma = cumdist;
				SvL_ma = cumdist;
				LoA_ma = {};
                V_releaseSvL_ma = 0;
                sections.erase(sections.begin()+i, sections.end());
				calculate_SvL();
                svl_shorten('c');
                break;
			}
		}
		cumdist += sections[i].length;
	}
	if (loa_timer) {
		if (!loa_timer->started)
			loa_timer->start(time_stamp);
		if (loa_timer->triggered()) {
			v_ema = 0;
			set_data();
            svl_shorten('n');
		}
	}
}

void calculate_SvL()
{
    if (mode != Mode::FS && mode != Mode::OS && mode != Mode::LS) {
        LoA = {};
        EoA = {};
        SvL = {};
        V_release = V_releaseSvL = 0;
        recalculate_MRSP();
        return;
    }

    LoA = MA->LoA_ma;
    EoA = MA->EoA_ma;
    SvL = MA->SvL_ma;
    V_releaseSvL = MA->V_releaseSvL_ma;

    for (auto it = mode_profiles.begin(); it != mode_profiles.end(); ++it) {
        if (it->start_SvL && mode != it->mode && ((LoA && LoA->first > it->start) || (SvL && *SvL > it->start)) && it->start > d_maxsafefront(it->start)) {
            EoA = SvL = it->start;
            LoA = {};
            V_releaseSvL = 0;
        }
    }

    for (auto it = level_crossings.begin(); it != level_crossings.end(); ++it) {
        if (!it->lx_protected && ((LoA && LoA->first > it->start) || (SvL && *SvL > it->start)) && !it->svl_replaced) {
            EoA = SvL = it->start;
            LoA = {};
            V_releaseSvL = 0;
            break;
        }
    }

    for (auto it = emergency_stops.begin(); it != emergency_stops.end(); ++it) {
        if (it->second && ((LoA && LoA->first > *it->second) || (SvL && *SvL > *it->second))) {
            EoA = SvL = *it->second;
            LoA = {};
            V_releaseSvL = 0;
        }
    }

    for (auto it = route_suitability.begin(); it != route_suitability.end(); ++it) {
        if ((LoA && LoA->first > it->second) || (SvL && *SvL > it->second)) {
            EoA = SvL = it->second;
            LoA = {};
            V_releaseSvL = 0;
        }
    }

    if (EoA && SvL) V_release = calculate_V_release();
    else V_release = 0;
    recalculate_MRSP();
}
void calculate_perturbation_location()
{
    d_perturbation_eoa = {};
    d_perturbation_svl = {};
    const std::map<distance, double> &mrsp = get_MRSP();
    if (MA && MA->EoA_ma) {
        target eoa(*MA->EoA_ma, 0, target_class::EoA);
        for (auto it = mrsp.begin(); it != mrsp.end() && !d_perturbation_eoa; ++it) {
            auto next = it;
            ++next;
            eoa.calculate_curves(it->second, 0, 0);
            distance d_I_eoa=eoa.d_I;
            distance start = it->first;
            distance end = next == mrsp.end() ? distance(std::numeric_limits<double>::max(), 0, 0) : next->first;
            if (d_I_eoa < start) {
                d_perturbation_eoa = start; // TODO: to be revised 
            } else if (d_I_eoa > start && d_I_eoa <= end) {
                d_perturbation_eoa = d_I_eoa;
            } else if (d_I_eoa > end && next != mrsp.end()) {
                eoa.calculate_curves(next->second, 0, 0);
                if (eoa.d_I < end)
                    d_perturbation_eoa = end;
            }
        }
    }
    if (MA && (MA->SvL_ma || MA->LoA_ma)) {
        target svl(MA->LoA_ma ? MA->LoA_ma->first : *MA->SvL_ma, MA->LoA_ma ? MA->LoA_ma->second : 0, MA->LoA_ma ? target_class::LoA : target_class::SvL);
        for (auto it = mrsp.begin(); it != mrsp.end() && !d_perturbation_svl; ++it) {
            auto next = it;
            ++next;
            svl.calculate_curves(it->second, 0, 0);
            distance d_I_svl=svl.d_I;
            distance start = it->first;
            distance end = next == mrsp.end() ? distance(std::numeric_limits<double>::max(), 0, 0) : next->first;
            if (d_I_svl > start && d_I_svl <= end) {
                d_perturbation_svl = d_I_svl;
            } else if (d_I_svl > end && next != mrsp.end()) {
                svl.calculate_curves(next->second, 0, 0);
                if (svl.d_I < end)
                    d_perturbation_svl = end;
            }
        }
    }
}
optional<movement_authority> MA;
std::set<speed_restriction> signal_speeds;
void set_data()
{
    MA->calculate_distances();
    calculate_SvL();
}
void replace_MA(movement_authority ma, bool coop)
{
    if (MA && MA->endtimer && MA->endtimer->started && ma.endtimer && ma.endtimer->startloc < d_maxsafefront(ma.endtimer->startloc)) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = MA->endtimer->start_time;
    } else if ((!MA || !MA->endtimer || !MA->endtimer->started) && ma.endtimer && ma.endtimer->startloc < d_maxsafefront(ma.endtimer->startloc)) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = get_milliseconds() - ma.endtimer->time;
    }
    if (MA && MA->ol && MA->ol->ovtimer && MA->ol->ovtimer->started && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc < d_maxsafefront(ma.ol->ovtimer->startloc)) {
        ma.ol->ovtimer->started = true;
        ma.ol->ovtimer->start_time = MA->ol->ovtimer->start_time;
    } else if ((!MA || !MA->ol || !MA->ol->ovtimer || !MA->ol->ovtimer->started) && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc < d_maxsafefront(ma.ol->ovtimer->startloc)) {
        ma.ol->ovtimer->started = true;
        ma.ol->ovtimer->start_time = get_milliseconds() - ma.ol->ovtimer->time;
    }
    optional<distance> prevSvL;
    optional<std::pair<distance,double>> prevLoA;
    if (MA) {
        prevSvL = MA->SvL_ma;
        prevLoA = MA->LoA_ma;
    }
    MA = ma;
    set_data();
    if (((prevSvL && MA->SvL_ma && *prevSvL>*MA->SvL_ma) || (MA->SvL_ma && prevLoA)) && !coop) {
        svl_shorten('b');
    }
}
void MA_infill(movement_authority ma)
{
    if (!MA)
        return;
    distance dist = ma.start;
    if (MA->get_end() < dist)
        return;

    if (MA->endtimer && MA->endtimer->started && ma.endtimer && ma.endtimer->startloc < d_maxsafefront(ma.endtimer->startloc)) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = MA->endtimer->start_time;
    } else if ((!MA->endtimer || !MA->endtimer->started) && ma.endtimer && ma.endtimer->startloc < d_maxsafefront(ma.endtimer->startloc)) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = get_milliseconds() - ma.endtimer->time;
    }
    if (MA->ol && MA->ol->ovtimer && MA->ol->ovtimer->started && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc < d_maxsafefront(ma.ol->ovtimer->startloc)) {
        ma.ol->ovtimer->started = true;
        ma.ol->ovtimer->start_time = MA->ol->ovtimer->start_time;
    } else if ((!MA->ol || !MA->ol->ovtimer || !MA->ol->ovtimer->started) && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc < d_maxsafefront(ma.ol->ovtimer->startloc)) {
        ma.ol->ovtimer->started = true;
        ma.ol->ovtimer->start_time = get_milliseconds() - ma.ol->ovtimer->time;
    }

    auto prevSvL = MA->SvL_ma;
    auto prevLoA = MA->LoA_ma;
    distance cumdist = MA->start;
    std::vector<ma_section> s;
    for (int i=0; i<MA->sections.size(); i++) {
        cumdist += MA->sections[i].length;
        if (cumdist >= dist) {
            s.insert(s.end(), MA->sections.begin(), MA->sections.begin()+i);
            ma_section ext = MA->sections[i];
            ext.length -= cumdist-dist;
            s.push_back(ext);
            break;
        }
    }
    s.insert(s.end(), ma.sections.begin(), ma.sections.end());
    MA->sections = s;
    MA->v_ema = ma.v_ema;
    MA->endtimer = ma.endtimer;
    MA->dp = ma.dp;
    MA->ol = ma.ol;
    MA->time_stamp = ma.time_stamp;
    set_data();
    if ((prevSvL && MA->SvL_ma && *prevSvL>*MA->SvL_ma) || (MA->SvL_ma && prevLoA)) {
        svl_shorten('b');
    }
}
void delete_MA()
{
    MA = {};
    calculate_SvL();
}
void delete_MA(distance eoa, distance svl)
{
    if (MA)
        MA->shorten(eoa, svl);
}
void set_signalling_restriction(movement_authority ma, bool infill)
{
    if (ma.v_main == 0)
        return;
    signal_speeds.clear();
    if (infill && MA && MA->v_main > 0)
        signal_speeds.insert(speed_restriction(MA->v_main, MA->start, ma.start, false));
    signal_speeds.insert(speed_restriction(ma.v_main, ma.start, ma.get_end(), false));
}
void movement_authority::shorten(distance eoa, distance svl)
{
    if (get_abs_end() < svl)
        return;
    if (get_end() < eoa)
        eoa = get_end();
    ol = {};
    dp = {svl-eoa, 0};
    v_main = 0;
    distance cum = start;
    for (int i=0; i<sections.size(); i++) {
        cum += sections[i].length;
        if (cum > eoa) {
            sections.erase(sections.begin()+i+1, sections.end());
            sections[i].length -= cum-eoa;
            if (sections[i].length <= 0)
                sections.erase(sections.begin()+i);
            break;
        }
    }
    set_data();
}
void movement_authority::reposition(distance current, double new_length)
{
    distance cum = start;
    for (int i=0; i<sections.size(); i++) {
        cum += sections[i].length;
        if (cum > current) {
            sections[i].length = current - cum + sections[i].length + new_length;
            break;
        }
    }
    set_data();
}
bool movement_authority::timers_to_expire(int64_t threshold)
{
	for (int i = 0; i < sections.size(); i++) {
		if (sections[i].stimer && sections[i].stimer->started && sections[i].stimer->remaining() < threshold)
            return true;
	}
	if (loa_timer && loa_timer->started && loa_timer->remaining() < threshold)
        return true;
	return false;
}