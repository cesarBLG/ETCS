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
optional<relocable_dist_base> d_perturbation_eoa;
optional<relocable_dist_base> d_perturbation_svl;
movement_authority::movement_authority(distance start, Level1_MA ma, int64_t time) : start(start), time_stamp(time)
{
    v_main = ma.V_MAIN.get_value();
    v_ema = ma.V_EMA.get_value();
    distance cumdist = start;
    for (int i=0; i<ma.N_ITER; i++) {
        ma_section section;
        section.length = ma.sections[i].L_SECTION.get_value(ma.Q_SCALE);
        if (ma.sections[i].Q_SECTIONTIMER && ma.sections[i].T_SECTIONTIMER != ma.sections[i].T_SECTIONTIMER.Infinity)
            section.stimer = section_timer(ma.sections[i].T_SECTIONTIMER, cumdist + ma.sections[i].D_SECTIONTIMERSTOPLOC.get_value(ma.Q_SCALE));
        sections.push_back(section);
        cumdist += section.length;
    }
    ma_section endsection;
    endsection.length = ma.L_ENDSECTION.get_value(ma.Q_SCALE);
    if (ma.Q_SECTIONTIMER && ma.T_SECTIONTIMER != ma.T_ENDTIMER.Infinity)
        endsection.stimer = section_timer(ma.T_SECTIONTIMER, cumdist + ma.D_SECTIONTIMERSTOPLOC.get_value(ma.Q_SCALE));
    sections.push_back(endsection);
    cumdist += endsection.length;
    if (ma.Q_ENDTIMER && ma.T_ENDTIMER != ma.T_ENDTIMER.Infinity)
        endtimer = end_timer(ma.T_ENDTIMER, cumdist-ma.D_ENDTIMERSTARTLOC.get_value(ma.Q_SCALE));
    if (ma.Q_OVERLAP) {
        overlap ov;
        ov.distance = ma.D_OL.get_value(ma.Q_SCALE);
        if (ma.V_RELEASEOL == ma.V_RELEASEOL.CalculateOnBoard)
            ov.vrelease = -2;
        else if (ma.V_RELEASEOL == ma.V_RELEASEOL.UseNationalValue)
            ov.vrelease = -1;
        else
            ov.vrelease = ma.V_RELEASEOL.get_value();
        if (ma.T_OL != ma.T_OL.Infinity)
            ov.ovtimer = end_timer(ma.T_OL, cumdist-ma.D_STARTOL.get_value(ma.Q_SCALE));
        ol = ov;
    }
    if (ma.Q_DANGERPOINT) {
        danger_point d;
        d.distance = ma.D_DP.get_value(ma.Q_SCALE);
        if (ma.V_RELEASEDP == ma.V_RELEASEDP.CalculateOnBoard)
            d.vrelease = -2;
        else if (ma.V_RELEASEDP == ma.V_RELEASEDP.UseNationalValue)
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
        if (ma.sections[i].Q_SECTIONTIMER && ma.sections[i].T_SECTIONTIMER != ma.sections[i].T_SECTIONTIMER.Infinity)
            section.stimer = section_timer(ma.sections[i].T_SECTIONTIMER, cumdist + ma.sections[i].D_SECTIONTIMERSTOPLOC.get_value(ma.Q_SCALE));
        sections.push_back(section);
        cumdist += section.length;
    }
    ma_section endsection;
    endsection.length = ma.L_ENDSECTION.get_value(ma.Q_SCALE);
    if (ma.Q_SECTIONTIMER && ma.T_SECTIONTIMER != ma.T_SECTIONTIMER.Infinity)
        endsection.stimer = section_timer(ma.T_SECTIONTIMER, cumdist + ma.D_SECTIONTIMERSTOPLOC.get_value(ma.Q_SCALE));
    sections.push_back(endsection);
    cumdist += endsection.length;
    if (ma.Q_ENDTIMER && ma.T_ENDTIMER != ma.T_SECTIONTIMER.Infinity)
        endtimer = end_timer(ma.T_ENDTIMER, cumdist-ma.D_ENDTIMERSTARTLOC.get_value(ma.Q_SCALE));
    if (ma.Q_OVERLAP) {
        overlap ov;
        ov.distance = ma.D_OL.get_value(ma.Q_SCALE);
        if (ma.V_RELEASEOL == ma.V_RELEASEOL.CalculateOnBoard)
            ov.vrelease = -2;
        else if (ma.V_RELEASEOL == ma.V_RELEASEOL.UseNationalValue)
            ov.vrelease = -1;
        else
            ov.vrelease = ma.V_RELEASEOL.get_value();
        if (ma.T_OL != ma.T_OL.Infinity)
            ov.ovtimer = end_timer(ma.T_OL, cumdist-ma.D_STARTOL.get_value(ma.Q_SCALE));
        ol = ov;
    }
    if (ma.Q_DANGERPOINT) {
        danger_point d;
        d.distance = ma.D_DP.get_value(ma.Q_SCALE);
        if (ma.V_RELEASEDP == ma.V_RELEASEDP.CalculateOnBoard)
            d.vrelease = -2;
        else if (ma.V_RELEASEDP == ma.V_RELEASEDP.UseNationalValue)
            d.vrelease = -1;
        else
            d.vrelease = ma.V_RELEASEDP.get_value();
        dp = d;
    }
}
void set_data();
void movement_authority::calculate_distances()
{
	distance end = get_end();
	if (v_ema == 0) {
		LoA_ma = {};
		EoA_ma = distance(end);
		if (ol) {
			SvL_ma = distance(end + ol->distance);
			V_releaseSvL_ma = ol->vrelease;
		} else if (dp) {
			SvL_ma = distance(end + dp->distance);
			V_releaseSvL_ma = dp->vrelease;
		} else {
			SvL_ma = distance(end);
			V_releaseSvL_ma = 0;
		}
	} else {
		EoA_ma = SvL_ma = {};
		LoA_ma = {end, v_ema};
	}
}
void movement_authority::update_timers()
{
	if (endtimer) {
		if (!endtimer->started && endtimer->startloc.max < d_maxsafefront(endtimer->startloc))
			endtimer->start();
		if (endtimer->triggered()) {
            endtimer = {};
            train_shorten('e');
		}
	}
	if (ol && ol->ovtimer) {
		if (!ol->ovtimer->started &&
			ol->ovtimer->startloc.max < d_maxsafefront(ol->ovtimer->startloc))
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
				sections[i].stimer->stoploc.min > d_minsafefront(sections[i].stimer->stoploc)) {
				sections[i].stimer->start(time_stamp);
			}
			if (sections[i].stimer->stoploc.min <= d_minsafefront(sections[i].stimer->stoploc)) {
				sections[i].stimer->started = false;
			}
			if (sections[i].stimer->triggered()) {
                v_ema = 0;
				EoA_ma = cumdist;
				SvL_ma = cumdist;
				LoA_ma = {};
                V_releaseSvL_ma = 0;
                sections.erase(sections.begin()+i, sections.end());
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
            loa_timer = {};
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
        if (it->start_SvL && mode != it->mode && ((LoA && LoA->first.min > it->start.max) || (SvL && SvL->max > it->start.max)) && it->start.max > d_maxsafefront(it->start)) {
            SvL = it->start;
            LoA = {};
            V_releaseSvL = 0;
        }
        if (mode != it->mode && (!EoA || EoA->est > it->start.est) && it->start.max > d_maxsafefront(it->start))
            EoA = it->start;
    }

    for (auto it = level_crossings.begin(); it != level_crossings.end(); ++it) {
        if (!it->lx_protected && ((LoA && LoA->first.min > it->start.max) || (SvL && SvL->max > it->start.max)) && !it->svl_replaced) {
            EoA = SvL = it->start;
            LoA = {};
            V_releaseSvL = 0;
        }
    }

    for (auto it = emergency_stops.begin(); it != emergency_stops.end(); ++it) {
        if (it->second && ((LoA && LoA->first.min > it->second->max) || (SvL && SvL->max > it->second->max))) {
            EoA = SvL = *it->second;
            LoA = {};
            V_releaseSvL = 0;
        }
    }

    for (auto it = route_suitability.begin(); it != route_suitability.end(); ++it) {
        if ((LoA && LoA->first.min > it->second.max) || (SvL && SvL->max > it->second.max)) {
            EoA = SvL = it->second;
            LoA = {};
            V_releaseSvL = 0;
        }
    }

    recalculate_MRSP();
    if (EoA && SvL) V_release = calculate_V_release();
    else V_release = 0;
}
void calculate_perturbation_location()
{
    d_perturbation_eoa = {};
    d_perturbation_svl = {};
    auto &mrsp = get_MRSP();
    if (MA && MA->EoA_ma) {
        relocable_dist_base targ = MA->EoA_ma->est;
        target eoa(targ, 0, target_class::EoA);
        for (auto it = mrsp.begin(); it != mrsp.end() && !d_perturbation_eoa; ++it) {
            auto next = it;
            ++next;
            eoa.calculate_curves(it->second, 0, 0);
            relocable_dist_base d_I_eoa = eoa.get_target_position();
            (dist_base&)d_I_eoa = eoa.d_I;
            relocable_dist_base start = it->first;
            relocable_dist_base end;
            if (next == mrsp.end())
                (dist_base&)end = dist_base::max;
            else
                end = next->first;
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
        relocable_dist_base targ = MA->LoA_ma ? MA->LoA_ma->first.max : MA->SvL_ma->max;
        target svl(targ, MA->LoA_ma ? MA->LoA_ma->second : 0, MA->LoA_ma ? target_class::LoA : target_class::SvL);
        for (auto it = mrsp.begin(); it != mrsp.end() && !d_perturbation_svl; ++it) {
            auto next = it;
            ++next;
            svl.calculate_curves(it->second, 0, 0);
            relocable_dist_base d_I_svl = svl.get_target_position();
            (dist_base&)d_I_svl = svl.d_I;
            relocable_dist_base start = it->first;
            relocable_dist_base end;
            if (next == mrsp.end())
                (dist_base&)end = dist_base::max;
            else
                end = next->first;
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
std::list<speed_restriction> signal_speeds;
void set_data()
{
    MA->calculate_distances();
}
void replace_MA(movement_authority ma, bool coop)
{
    if (MA && MA->endtimer && MA->endtimer->started && ma.endtimer && ma.endtimer->startloc.max < d_maxsafefront(ma.endtimer->startloc)) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = MA->endtimer->start_time;
    } else if ((!MA || !MA->endtimer || !MA->endtimer->started) && ma.endtimer && ma.endtimer->startloc.max < d_maxsafefront(ma.endtimer->startloc)) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = get_milliseconds() - ma.endtimer->time;
    }
    if (MA && MA->ol && MA->ol->ovtimer && MA->ol->ovtimer->started && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc.max < d_maxsafefront(ma.ol->ovtimer->startloc)) {
        ma.ol->ovtimer->started = true;
        ma.ol->ovtimer->start_time = MA->ol->ovtimer->start_time;
    } else if ((!MA || !MA->ol || !MA->ol->ovtimer || !MA->ol->ovtimer->started) && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc.max < d_maxsafefront(ma.ol->ovtimer->startloc)) {
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
    if (((prevSvL && MA->SvL_ma && prevSvL->max>MA->SvL_ma->max) || (MA->SvL_ma && prevLoA)) && !coop) {
        svl_shorten('b');
    }
}
void MA_infill(movement_authority ma)
{
    if (!MA)
        return;
    dist_base dist = ma.start.min;
    if (MA->get_end().max < dist)
        return;

    if (MA->endtimer && MA->endtimer->started && ma.endtimer && ma.endtimer->startloc.max < d_maxsafefront(ma.endtimer->startloc)) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = MA->endtimer->start_time;
    } else if ((!MA->endtimer || !MA->endtimer->started) && ma.endtimer && ma.endtimer->startloc.max < d_maxsafefront(ma.endtimer->startloc)) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = get_milliseconds() - ma.endtimer->time;
    }
    if (MA->ol && MA->ol->ovtimer && MA->ol->ovtimer->started && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc.max < d_maxsafefront(ma.ol->ovtimer->startloc)) {
        ma.ol->ovtimer->started = true;
        ma.ol->ovtimer->start_time = MA->ol->ovtimer->start_time;
    } else if ((!MA->ol || !MA->ol->ovtimer || !MA->ol->ovtimer->started) && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc.max < d_maxsafefront(ma.ol->ovtimer->startloc)) {
        ma.ol->ovtimer->started = true;
        ma.ol->ovtimer->start_time = get_milliseconds() - ma.ol->ovtimer->time;
    }

    auto prevSvL = MA->SvL_ma;
    auto prevLoA = MA->LoA_ma;
    distance cumdist = MA->start;
    std::vector<ma_section> s;
    for (int i=0; i<MA->sections.size(); i++) {
        cumdist += MA->sections[i].length;
        if (cumdist.min >= dist) {
            s.insert(s.end(), MA->sections.begin(), MA->sections.begin()+i);
            ma_section ext = MA->sections[i];
            ext.length -= cumdist.min-dist;
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
    if ((prevSvL && MA->SvL_ma && prevSvL->max>MA->SvL_ma->max) || (MA->SvL_ma && prevLoA)) {
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
        signal_speeds.push_back(speed_restriction(MA->v_main, MA->start, ma.start, false));
    signal_speeds.push_back(speed_restriction(ma.v_main, ma.start, ma.get_end(), false));
}
void movement_authority::shorten(distance eoa, distance svl)
{
    if (get_abs_end().min < svl.max)
        return;
    if (get_end().min < eoa.est)
        eoa = get_end();
    ol = {};
    dp = {svl.max-eoa.est, 0};
    v_main = 0;
    distance cum = start;
    for (int i=0; i<sections.size(); i++) {
        cum += sections[i].length;
        if (cum.min > eoa.est) {
            sections.erase(sections.begin()+i+1, sections.end());
            sections[i].length -= cum.min-eoa.est;
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
        if (cum.est > current.est) {
            sections[i].length = current.est - cum.est + sections[i].length + new_length;
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