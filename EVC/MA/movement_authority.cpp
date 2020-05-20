/*
 * European Train Control System
 * Copyright (C) 2019  César Benito <cesarbema2009@hotmail.com>
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

#include "movement_authority.h"
#include "../Supervision/supervision.h"
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
        if (ma.V_RELEASEOL == V_RELEASE_t::CalculateOnBoard)
            ov.vrelease = -2;
        else if (ma.V_RELEASEOL == V_RELEASE_t::UseNationalValue)
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
        if (ma.V_RELEASEDP == V_RELEASE_t::CalculateOnBoard)
            d.vrelease = -2;
        else if (ma.V_RELEASEDP == V_RELEASE_t::UseNationalValue)
            d.vrelease = -1;
        else
            d.vrelease = ma.V_RELEASEDP.get_value();
        dp = d;
    }
}
void set_data();
void movement_authority::update_timers()
{
	if (endtimer) {
		if (!endtimer->started && endtimer->startloc < d_maxsafefront(endtimer->startloc.get_reference()))
			endtimer->start();
		if (endtimer->triggered()) {
			V_release = 0;
			EoA = d_estfront;
			LoA = {};
			recalculate_MRSP();
		}
	}
	if (ol && ol->ovtimer) {
		if (!ol->ovtimer->started &&
			ol->ovtimer->startloc < d_maxsafefront(ol->ovtimer->startloc.get_reference()))
			ol->ovtimer->start();
		if (ol->ovtimer->triggered() || (ol->ovtimer->started && V_est <= 0)) {
			v_ema = 0;
			ol = {};
			set_data();
		}
	}
	distance cumdist = start;
	for (int i = 0; i < sections.size(); i++) {
		if (sections[i].stimer) {
			if (!sections[i].stimer->started &&
				sections[i].stimer->stoploc > d_minsafefront(sections[i].stimer->stoploc.get_reference())) {
				sections[i].stimer->start(time_stamp);
			}
			if (sections[i].stimer->stoploc <= d_minsafefront(sections[i].stimer->stoploc.get_reference())) {
				sections[i].stimer->started = false;
			}
			if (sections[i].stimer->triggered()) {
                v_ema = 0;
				EoA = cumdist;
				SvL = cumdist;
				LoA = {};
				recalculate_MRSP();
                sections.erase(sections.begin()+i, sections.end());
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
		}
	}
}

optional<movement_authority> MA;
std::set<speed_restriction> signal_speeds;
void set_data()
{
    distance end = MA->get_end();
    if (MA->v_ema == 0) {
        LoA = {};
        EoA = distance(end);
        if (MA->ol) {
            SvL = distance(end+MA->ol->distance);
            V_releaseSvL = MA->ol->vrelease;
        } else if (MA->dp) {
            SvL = distance(end+MA->dp->distance);
            V_releaseSvL = MA->dp->vrelease;
        } else {
            SvL = distance(end);
            V_releaseSvL = 0;
        }
    } else {
        EoA = SvL = {};
        LoA = {end,MA->v_ema};
    }
    if (MA->v_ema == 0)
        V_release = calculate_V_release();
    recalculate_MRSP();
}
void replace_MA(movement_authority ma)
{
    if (MA && MA->endtimer && MA->endtimer->started && ma.endtimer && ma.endtimer->startloc < d_maxsafefront(ma.endtimer->startloc.get_reference())) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = MA->endtimer->start_time;
    } else if ((!MA || !MA->endtimer || !MA->endtimer->started) && ma.endtimer && ma.endtimer->startloc < d_maxsafefront(ma.endtimer->startloc.get_reference())) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = get_milliseconds() - ma.endtimer->time;
    }
    if (MA && MA->ol && MA->ol->ovtimer && MA->ol->ovtimer->started && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc < d_maxsafefront(ma.ol->ovtimer->startloc.get_reference())) {
        ma.ol->ovtimer->started = true;
        ma.ol->ovtimer->start_time = MA->ol->ovtimer->start_time;
    } else if ((!MA || !MA->ol || !MA->ol->ovtimer || !MA->ol->ovtimer->started) && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc < d_maxsafefront(ma.ol->ovtimer->startloc.get_reference())) {
        ma.ol->ovtimer->started = true;
        ma.ol->ovtimer->start_time = get_milliseconds() - ma.ol->ovtimer->time;
    }
    auto prevSvL = SvL;
    auto prevLoA = LoA;
    MA = ma;
    set_data();
    if ((prevSvL && SvL && *prevSvL>*SvL) || (SvL && prevLoA)) {
        //MA shortening
    }
}
void MA_infill(movement_authority ma)
{
    if (!MA)
        return;
    distance dist = ma.start;
    if (MA->get_end() < dist)
        return;

    
    if (MA->endtimer && MA->endtimer->started && ma.endtimer && ma.endtimer->startloc < d_maxsafefront(ma.endtimer->startloc.get_reference())) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = MA->endtimer->start_time;
    } else if ((!MA->endtimer || !MA->endtimer->started) && ma.endtimer && ma.endtimer->startloc < d_maxsafefront(ma.endtimer->startloc.get_reference())) {
        ma.endtimer->started = true;
        ma.endtimer->start_time = get_milliseconds() - ma.endtimer->time;
    }
    if (MA->ol && MA->ol->ovtimer && MA->ol->ovtimer->started && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc < d_maxsafefront(ma.ol->ovtimer->startloc.get_reference())) {
        ma.ol->ovtimer->started = true;
        ma.ol->ovtimer->start_time = MA->ol->ovtimer->start_time;
    } else if ((!MA->ol || !MA->ol->ovtimer || !MA->ol->ovtimer->started) && ma.ol && ma.ol->ovtimer && ma.ol->ovtimer->startloc < d_maxsafefront(ma.ol->ovtimer->startloc.get_reference())) {
        ma.ol->ovtimer->started = true;
        ma.ol->ovtimer->start_time = get_milliseconds() - ma.ol->ovtimer->time;
    }

    auto prevSvL = SvL;
    auto prevLoA = LoA;
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
    if ((prevSvL && SvL && *prevSvL>*SvL) || (SvL && prevLoA)) {
        //MA shortening
    }
}
void delete_MA()
{
    MA = {};
    SvL = {};
    EoA = {};
    LoA = {};
    V_release = 0;
}
void set_signalling_restriction(movement_authority ma, bool infill)
{
    if (ma.v_main == 0)
        return;
    signal_speeds.clear();
    if (infill && MA)
        signal_speeds.insert(speed_restriction(MA->v_main, MA->start, ma.start, false));
    signal_speeds.insert(speed_restriction(ma.v_main, ma.start, ma.get_end(), false));
}