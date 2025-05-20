/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "mode_profile.h"
#include "../Procedures/mode_transition.h"
std::list<mode_profile> mode_profiles;
bool in_mode_ack_area;
bool mode_timer_started = false;
int64_t mode_timer;
optional<mode_profile> requested_mode_profile;
double lssma;
bool display_lssma;
optional<int64_t> display_lssma_time;
bool ls_function_marker;
void update_mode_profile()
{
    if (mode_timer_started && mode_timer + T_ACK*1000 < get_milliseconds()) {
        mode_timer_started = false;
        brake_conditions.push_back({-1, nullptr, [](brake_command_information &i) {
            if (mode_acknowledged || mode_to_ack != mode || !mode_acknowledgeable)
                return true;
            return false;
        }});
    }
    in_mode_ack_area = false;
    if (mode_profiles.empty()) {
        requested_mode_profile = {};
        return;
    }
    mode_profile first = mode_profiles.front();
    if (first.length < std::numeric_limits<float>::max() && first.start.min + first.length < d_minsafefront(first.start)) {
        mode_profiles.pop_front();
        update_mode_profile();
        calculate_SvL();
        return;
    }
    if (mode_profiles.size() > 1) {
        mode_profile second = *(++mode_profiles.begin());
        if (second.start.max < d_maxsafefront(second.start)) {
            mode_profiles.pop_front();
            update_mode_profile();
            calculate_SvL();
            return;
        }
    }
    for (auto it = mode_profiles.begin(); it != mode_profiles.end(); ++it) {
        mode_profile &p = *it;
        if (d_estfront > p.start.est-p.acklength) {
            if (mode_acknowledged && mode_to_ack == p.mode) {
                if (it != mode_profiles.begin()) {
                    mode_profiles.erase(mode_profiles.begin(), it);
                    calculate_SvL();
                }
                return;
            }
            in_mode_ack_area = true;
            requested_mode_profile = p;
            if (mode != p.mode && V_est < p.speed) {
                mode_acknowledgeable = true;
                mode_acknowledged = false;
                mode_to_ack = first.mode;
                break;
            }
        }
    }
    if (d_maxsafefront(first.start) > first.start.max) {
        requested_mode_profile = first;
        if (mode == first.mode) {
            if (mode == Mode::OS && (!OS_speed || OS_speed->speed != requested_mode_profile->speed)) {
                OS_speed = speed_restriction(requested_mode_profile->speed, distance(std::numeric_limits<double>::lowest(), 0, 0), distance(std::numeric_limits<double>::max(), 0, 0), false);
                recalculate_MRSP();
            } else if (mode == Mode::SH && (!SH_speed || SH_speed->speed != requested_mode_profile->speed)) {
                SH_speed = speed_restriction(requested_mode_profile->speed, distance::from_odometer(dist_base::min), distance::from_odometer(dist_base::max), false);
                recalculate_MRSP();
            }
        } else {
            mode_timer_started = true;
            mode_timer = get_milliseconds();
            mode_acknowledgeable = true;
            mode_acknowledged = false;
            mode_to_ack = first.mode;
        }
    }

    if (requested_mode_profile && requested_mode_profile->mode == Mode::LS) {
        if (LoA) {
            lssma = LoA->second;
            auto mrsp = get_MRSP();
            for (auto it = mrsp.begin(); it != mrsp.end(); ++it) {
                if (d_minsafefront(it->first) < it->first && lssma > it->second)
                    lssma = it->second;
            }
        } else {
            lssma = 0;
        }
        if (ls_function_marker)
            display_lssma = display_lssma_time && get_milliseconds()>*display_lssma_time;
        else
            display_lssma = (EoA || (LoA && LoA->second < requested_mode_profile->speed)) && lssma < V_train;
    }
}
void reset_mode_profile(distance ref, bool infill)
{
    if (infill) {
        for (auto it = mode_profiles.begin(); it != mode_profiles.end(); ++it) {
            if (it->start.max >= ref.max) {
                mode_profiles.erase(it, mode_profiles.end());
                break;
            }
        }
    } else {
        mode_profiles.clear();
    }
    if ((mode_to_ack == Mode::OS || mode_to_ack == Mode::LS || mode_to_ack == Mode::SH) && mode_to_ack != mode) {
        mode_acknowledgeable = mode_acknowledged = false;
    }
}
void set_mode_profile(ModeProfile profile, distance ref, bool infill)
{
    reset_mode_profile(ref, infill);
    std::vector<MP_element_packet> mps;
    mps.push_back(profile.element);
    mps.insert(mps.end(), profile.elements.begin(), profile.elements.end());
    distance start = ref;
    for (auto it = mps.begin(); it != mps.end(); ++it) {
        start += it->D_MAMODE.get_value(profile.Q_SCALE);
        mode_profile p;
        p.start = start;
        p.length = it->L_MAMODE == it->L_MAMODE.Infinity ? std::numeric_limits<float>::max() : it->L_MAMODE.get_value(profile.Q_SCALE);
        p.acklength = it->L_ACKMAMODE.get_value(profile.Q_SCALE);
        if (it->M_MAMODE == it->M_MAMODE.OS) {
            p.mode = Mode::OS;
            p.speed = V_NVONSIGHT;
        } else if (it->M_MAMODE == it->M_MAMODE.LS) {
            p.mode = Mode::LS;
            p.speed = V_NVLIMSUPERV;
        } else if (it->M_MAMODE == it->M_MAMODE.SH) {
            p.mode = Mode::SH;
            p.speed = V_NVSHUNT;
        }
        p.start_SvL = it->Q_MAMODE == it->Q_MAMODE.BeginningIsSvL;
        if (it->V_MAMODE != it->V_MAMODE.UseNationalValue)
            p.speed = it->V_MAMODE.get_value();
        mode_profiles.push_back(p);
    }
}