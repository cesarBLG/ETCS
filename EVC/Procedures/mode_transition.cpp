/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "mode_transition.h"
#include "train_trip.h"
#include "override.h"
#include "start.h"
#include "../Supervision/emergency_stop.h"
#include "../Supervision/track_pbd.h"
#include "../Position/linking.h"
#include "../DMI/text_message.h"
#include "../DMI/track_ahead_free.h"
#include "level_transition.h"
#include "../TrackConditions/track_condition.h"
#include "../TrackConditions/route_suitability.h"
#include "../Packets/radio.h"
#include "../Euroradio/session.h"
#include "../LX/level_crossing.h"
#include "../language/language.h"
#include "../TrainSubsystems/train_interface.h"
#include "../TrainSubsystems/cold_movement.h"
#include "../Procedures/reversing.h"
#include <map>
#include "platform_runtime.h"
cond mode_conditions[78];
static std::vector<mode_transition> ordered_transitions[20];
Mode mode=Mode::NP;
int64_t last_mode_change;
bool entering_mode_message_is_time_dependent = false;
bool mode_acknowledgeable=false;
bool mode_acknowledged=false;
Mode mode_to_ack;
optional<std::set<bg_id>> sh_balises;
optional<std::set<bg_id>> sr_balises;
void set_mode_deleted_data();
void initialize_mode_transitions()
{
    cond *c = mode_conditions;
    c[1] = [](){return isolated;};
    c[2] = [](){return cab_active[0] ^ cab_active[1];};
    c[3] = [](){return !sl_signal && V_est == 0;};
    c[4] = [](){return true;};
    c[7] = [](){return level!=Level::N0 && level!=Level::NTC && V_est==0 && mode_to_ack==Mode::TR && mode_acknowledged;};
    c[8] = [](){return mode_to_ack==Mode::SR && mode_acknowledged;};
    c[10] = [](){return train_data_valid && MA && SSP_begin() < dist_base::max && !get_gradient().empty() && !requested_mode_profile;};
    c[12] = [](){return level == Level::N1 && MA && ((MA->EoA_ma && MA->EoA_ma->min<(d_minsafefront(*MA->EoA_ma)-L_antenna_front)) || (MA->LoA_ma && MA->LoA_ma->first.min<(d_minsafefront(MA->LoA_ma->first)-L_antenna_front)));};
    c[14] = [](){return !cab_active[0] && !cab_active[1] && V_est == 0 && sl_signal;};
    c[15] = [](){return mode_to_ack==Mode::OS && mode_acknowledged;};
    c[16] = [](){return (level == Level::N2 || level==Level::N3) && MA && ((MA->EoA_ma && MA->EoA_ma->min<d_minsafefront(*MA->EoA_ma)) || (MA->LoA_ma && MA->LoA_ma->first.min<d_minsafefront(MA->LoA_ma->first)));};
    c[21] = [](){return level == Level::N0;};
    c[25] = [](){return (level == Level::N1 || level == Level::N2 || level==Level::N3) && MA && SSP_begin() < dist_base::max && !get_gradient().empty() && !requested_mode_profile;};
    c[27] = [](){return !cab_active[0] && !cab_active[1];};
    c[28] = [](){return !cab_active[0] && !cab_active[1];};
    c[29] = [](){return false;};
    c[30] = [](){return !cab_active[0] && !cab_active[1] && !ps_signal;};
    c[31] = [](){return MA && SSP_begin() < dist_base::max && !get_gradient().empty() && (level == Level::N2 || level==Level::N3) && !requested_mode_profile;};
    c[32] = [](){return MA && SSP_begin() < dist_base::max && !get_gradient().empty() && level == Level::N1 && MA->get_v_main() > 0 && !requested_mode_profile;};
    c[34] = [](){return !mode_profiles.empty() && mode_profiles.front().mode == Mode::OS && mode_profiles.front().start.max < d_maxsafefront(mode_profiles.front().start) && (level == Level::N1 || level == Level::N2 || level==Level::N3);};
    c[37] = [](){return false;};
    c[39] = [](){return (level == Level::N1 || level == Level::N2 || level==Level::N3) && !MA;};
    c[40] = [](){return !mode_profiles.empty() && mode_profiles.front().mode == Mode::OS && mode_profiles.front().start.max < d_maxsafefront(mode_profiles.front().start);};
    c[42] = [](){return SR_dist && SR_dist->est < d_estfront && !overrideProcedure;};
    c[43] = [](){return !overrideProcedure && formerEoA && formerEoA->min<d_minsafefront(*formerEoA)-L_antenna_front;};
    c[44] = [](){return overrideProcedure && level == Level::N1;};
    c[45] = [](){
        if (overrideProcedure && (level == Level::N2 || level == Level::N3)) {
            for (auto it = emergency_stops.begin(); it != emergency_stops.end(); ++it) {
                if (!it->second)
                    return false;
            }
            return true;
        }
        return false;
    };
    c[50] = [](){return mode_to_ack==Mode::SH && mode_acknowledged;};
    c[51] = [](){return !mode_profiles.empty() && mode_profiles.front().mode == Mode::SH && mode_profiles.front().start.max < d_maxsafefront(mode_profiles.front().start);};
    c[56] = [](){return level==Level::NTC;};
    c[58] = [](){return level==Level::NTC && mode_to_ack == Mode::SN && mode_acknowledged;};
    c[59] = [](){return V_est == 0 && mode_to_ack == Mode::RV && mode_acknowledged;};
    c[60] = [](){return mode_to_ack==Mode::UN && mode_acknowledged;};
    c[61] = [](){return !mode_profiles.empty() && mode_profiles.front().mode == Mode::SH && mode_profiles.front().start.max < d_maxsafefront(mode_profiles.front().start) && (level == Level::N1 || level == Level::N2 || level==Level::N3);};
    c[62] = [](){return level==Level::N0 && V_est==0 && train_data_valid && mode_acknowledged;};
    c[63] = [](){return level==Level::NTC && V_est==0 && train_data_valid && mode_acknowledged;};
    c[68] = [](){return (level==Level::N0 || level==Level::NTC) && V_est==0 && !train_data_valid && mode_acknowledged;};
    c[69] = [](){return SSP_begin() > d_estfront || get_gradient().empty() || get_gradient().begin()->first>d_estfront;};
    c[70] = [](){return mode_to_ack==Mode::LS && mode_acknowledged;};
    c[71] = [](){return !mode_profiles.empty() && mode_profiles.front().mode == Mode::LS && mode_profiles.front().start.max < d_maxsafefront(mode_profiles.front().start)  && (level == Level::N1 || level == Level::N2 || level==Level::N3);};
    c[72] = [](){return !mode_profiles.empty() && mode_profiles.front().mode == Mode::LS && mode_profiles.front().start.max < d_maxsafefront(mode_profiles.front().start);};
    c[73] = [](){return !(in_mode_ack_area && *in_mode_ack_area == Mode::LS) && !mode_profiles.empty() && mode_profiles.front().mode == Mode::OS && mode_profiles.front().start.max < d_maxsafefront(mode_profiles.front().start);};
    c[74] = [](){return !(in_mode_ack_area && *in_mode_ack_area == Mode::OS) && !mode_profiles.empty() && mode_profiles.front().mode == Mode::LS && mode_profiles.front().start.max < d_maxsafefront(mode_profiles.front().start);};
    
    // Out of SRS conditions
    c[75] = [](){return !isolated;};
    c[76] = [](){return failed; };
    c[77] = [](){return !failed;};

    std::vector<mode_transition> transitions;
    transitions.push_back({Mode::SB, Mode::SR, {8,37}, 7});
    transitions.push_back({Mode::FS, Mode::SR, {37}, 6});
    transitions.push_back({Mode::LS, Mode::SR, {37}, 6});
    transitions.push_back({Mode::OS, Mode::SR, {37}, 6});
    transitions.push_back({Mode::PT, Mode::SR, {8,37}, 5});
    transitions.push_back({Mode::UN, Mode::SR, {44,45}, 4});
    transitions.push_back({Mode::SN, Mode::SR, {44,45}, 4});

    transitions.push_back({Mode::SB, Mode::FS, {10}, 7});
    transitions.push_back({Mode::LS, Mode::FS, {31,32}, 6});
    transitions.push_back({Mode::SR, Mode::FS, {31,32}, 6});
    transitions.push_back({Mode::OS, Mode::FS, {31,32}, 6});
    transitions.push_back({Mode::UN, Mode::FS, {25}, 7});
    transitions.push_back({Mode::PT, Mode::FS, {31}, 5});
    transitions.push_back({Mode::SN, Mode::FS, {25}, 7});

    transitions.push_back({Mode::SB, Mode::TR, {20}, 4});
    transitions.push_back({Mode::SH, Mode::TR, {49,52,65}, 4});
    transitions.push_back({Mode::FS, Mode::TR, {12,16,17,18,20,41,65,66,69}, 4});
    transitions.push_back({Mode::LS, Mode::TR, {12,16,17,18,20,41,65,66,69}, 4});
    transitions.push_back({Mode::SR, Mode::TR, {18,20,42,43,36,54,65}, 4});
    transitions.push_back({Mode::OS, Mode::TR, {12,16,17,18,20,41,65,66,69}, 4});
    transitions.push_back({Mode::UN, Mode::TR, {20,39,67}, 5});
    transitions.push_back({Mode::SN, Mode::TR, {20,35,38,39,67}, 5});

    transitions.push_back({Mode::TR, Mode::PT, {7}, 4});

    transitions.push_back({Mode::SB, Mode::UN, {60}, 7});
    transitions.push_back({Mode::FS, Mode::UN, {21}, 6});
    transitions.push_back({Mode::LS, Mode::UN, {21}, 6});
    transitions.push_back({Mode::SR, Mode::UN, {21}, 6});
    transitions.push_back({Mode::OS, Mode::UN, {21}, 6});
    transitions.push_back({Mode::TR, Mode::UN, {62}, 4});
    transitions.push_back({Mode::SN, Mode::UN, {21}, 7});

    transitions.push_back({Mode::SB, Mode::SH, {5,6,50}, 7});
    transitions.push_back({Mode::PS, Mode::SH, {23}, 4});
    transitions.push_back({Mode::FS, Mode::SH, {5,6,50,51}, 6});
    transitions.push_back({Mode::LS, Mode::SH, {5,6,50,51}, 6});
    transitions.push_back({Mode::SR, Mode::SH, {5,6,51}, 6});
    transitions.push_back({Mode::OS, Mode::SH, {5,6,50,51}, 6});
    transitions.push_back({Mode::UN, Mode::SH, {5,61}, 7});
    transitions.push_back({Mode::TR, Mode::SH, {68}, 4});
    transitions.push_back({Mode::PT, Mode::SH, {5,6,50}, 5});
    transitions.push_back({Mode::SN, Mode::SH, {5,61}, 7});

    transitions.push_back({Mode::NP, Mode::SB, {4}, 2});
    transitions.push_back({Mode::PS, Mode::SB, {22}, 4});
    transitions.push_back({Mode::SH, Mode::SB, {19,27,30}, 5});
    transitions.push_back({Mode::FS, Mode::SB, {28}, 5});
    transitions.push_back({Mode::LS, Mode::SB, {28}, 5});
    transitions.push_back({Mode::SR, Mode::SB, {28}, 5});
    transitions.push_back({Mode::OS, Mode::SB, {28}, 5});
    transitions.push_back({Mode::SL, Mode::SB, {2,3}, 3});
    transitions.push_back({Mode::NL, Mode::SB, {28,47}, 3});
    transitions.push_back({Mode::UN, Mode::SB, {28}, 6});
    transitions.push_back({Mode::PT, Mode::SB, {28}, 4});
    transitions.push_back({Mode::SN, Mode::SB, {28}, 6});
    transitions.push_back({Mode::RV, Mode::SB, {28}, 4});

    transitions.push_back({Mode::SB, Mode::OS, {15}, 7});
    transitions.push_back({Mode::FS, Mode::OS, {15, 40}, 6});
    transitions.push_back({Mode::LS, Mode::OS, {15, 73}, 6});
    transitions.push_back({Mode::SR, Mode::OS, {40}, 6});
    transitions.push_back({Mode::UN, Mode::OS, {34}, 7});
    transitions.push_back({Mode::PT, Mode::OS, {15}, 5});
    transitions.push_back({Mode::SN, Mode::OS, {34}, 7});

    transitions.push_back({Mode::SB, Mode::SL, {14}, 5});
    transitions.push_back({Mode::PS, Mode::SL, {14}, 4});

    transitions.push_back({Mode::FS, Mode::UN, {59}, 6});
    transitions.push_back({Mode::LS, Mode::UN, {59}, 6});
    transitions.push_back({Mode::OS, Mode::UN, {59}, 6});

    transitions.push_back({Mode::SB, Mode::SN, {58}, 7});
    transitions.push_back({Mode::FS, Mode::SN, {56}, 6});
    transitions.push_back({Mode::LS, Mode::SN, {56}, 6});
    transitions.push_back({Mode::SR, Mode::SN, {56}, 6});
    transitions.push_back({Mode::OS, Mode::SN, {56}, 6});
    transitions.push_back({Mode::UN, Mode::SN, {56}, 7});
    transitions.push_back({Mode::TR, Mode::SN, {63}, 4});

    transitions.push_back({Mode::SB, Mode::LS, {70}, 7});
    transitions.push_back({Mode::FS, Mode::LS, {70, 72}, 6});
    transitions.push_back({Mode::SR, Mode::LS, {72}, 6});
    transitions.push_back({Mode::OS, Mode::LS, {70, 74}, 6});
    transitions.push_back({Mode::UN, Mode::LS, {71}, 7});
    transitions.push_back({Mode::PT, Mode::LS, {70}, 5});
    transitions.push_back({Mode::SN, Mode::LS, {71}, 7});

    transitions.push_back({Mode::NP, Mode::IS, {1}, 1});
    transitions.push_back({Mode::SB, Mode::IS, {1}, 1});
    transitions.push_back({Mode::PS, Mode::IS, {1}, 1});
    transitions.push_back({Mode::SH, Mode::IS, {1}, 1});
    transitions.push_back({Mode::FS, Mode::IS, {1}, 1});
    transitions.push_back({Mode::LS, Mode::IS, {1}, 1});
    transitions.push_back({Mode::SR, Mode::IS, {1}, 1});
    transitions.push_back({Mode::OS, Mode::IS, {1}, 1});
    transitions.push_back({Mode::SL, Mode::IS, {1}, 1});
    transitions.push_back({Mode::NL, Mode::IS, {1}, 1});
    transitions.push_back({Mode::UN, Mode::IS, {1}, 1});
    transitions.push_back({Mode::TR, Mode::IS, {1}, 1});
    transitions.push_back({Mode::PT, Mode::IS, {1}, 1});
    transitions.push_back({Mode::SF, Mode::IS, {1}, 1});
    transitions.push_back({Mode::SN, Mode::IS, {1}, 1});
    transitions.push_back({Mode::RV, Mode::IS, {1}, 1});

    transitions.push_back({Mode::SB, Mode::NL, {46}, 6});
    transitions.push_back({Mode::SH, Mode::NL, {46}, 5});
    transitions.push_back({Mode::FS, Mode::NL, {46}, 6});
    transitions.push_back({Mode::LS, Mode::NL, {46}, 6});
    transitions.push_back({Mode::SR, Mode::NL, {46}, 6});
    transitions.push_back({Mode::OS, Mode::NL, {46}, 6});

    // Out of SRS conditions
    transitions.push_back({ Mode::IS, Mode::SB, {75}, 1 });
    transitions.push_back({ Mode::NP, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::SB, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::PS, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::SH, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::FS, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::LS, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::SR, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::OS, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::SL, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::NL, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::UN, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::TR, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::PT, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::SF, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::SN, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::RV, Mode::SF, {76}, 1 });
    transitions.push_back({ Mode::SF, Mode::SB, {77}, 1 });

    for (mode_transition &t : transitions) {
        ordered_transitions[(int)t.from].push_back(t);
    }
    set_mode_deleted_data();
}

struct stored_information
{
    std::function<void()> delete_info;
    std::function<void()> invalidate_info;
};
static std::map<int,stored_information> information_list;
struct deleted_information
{
    int index;
    bool deleted;
    bool invalidated;
    std::set<int> exceptions;
    void handle(Mode from, Mode to)
    {
        if (deleted && information_list[index].delete_info != nullptr) {
            if (exceptions.find(1) != exceptions.end() && from == Mode::SH)
                return;
            if (exceptions.find(2) != exceptions.end() && from == Mode::PS)
                return;
            information_list[index].delete_info();
        }
        if (invalidated && information_list[index].invalidate_info != nullptr) {
            information_list[index].invalidate_info();
        }
    }
};
std::map<Mode, std::vector<deleted_information>> deleted_informations;
bool prev_desk_open;
void update_mode_status()
{
    if (!prev_desk_open && (cab_active[0] ^ cab_active[1])) {
        odometer_orientation = cab_active[0] ? 1 : -1;
        extern optional<distance> standstill_position;
        standstill_position = {};
        if (solr) {
            if (solr->dir != -1 && solr->position.orientation != odometer_orientation)
                solr->dir = 1-solr->dir;
            solr->position.orientation = odometer_orientation;
        }
        for (auto &it : orbgs) {
            if (it.first.dir != -1 && it.first.position.orientation != odometer_orientation)
                it.first.dir = 1-it.first.dir;
            it.first.position.orientation = odometer_orientation;
        }
        update_odometer();
        void reset_eurobalise_data();
        reset_eurobalise_data();
    }
    prev_desk_open = cab_active[0] ^ cab_active[1];
    update_mode_profile();
    std::vector<mode_transition> &available=ordered_transitions[(int)mode];
    int priority = 10;
    int transition_index=-1;
    Mode transition = mode;
    for (mode_transition &t : available) {
        int i = t.happens();
        if (t.from == mode && i>=0 && t.priority < priority) {
#ifdef DEBUG_MODE
            platform->debug_print("Mode transition: "+std::to_string(i));
#endif
            transition = t.to;
            priority = t.priority;
            transition_index = i;
        }
    }
    if (mode != transition) {
        mode_acknowledged = false;
        if (transition != mode_to_ack) mode_acknowledgeable = false;
        Mode prevmode = mode;
        mode = transition;
        last_mode_change = get_milliseconds();
        if (mode == Mode::TR || mode == Mode::LS || mode == Mode::OS || mode == Mode::SH)
            overrideProcedure = false;
        if (mode == Mode::SR) {
            SR_speed = speed_restriction(V_NVSTFF, distance::from_odometer(dist_base::min), distance::from_odometer(dist_base::max), false);
            SR_dist_start = distance::from_odometer(d_estfront_dir[odometer_orientation == -1]);
        } else {
            SR_speed_override = {};
            SR_speed = {};
            SR_dist_override = {};
            SR_dist_start = {};
            SR_dist = {};
            formerEoA = {};
        }
        if (mode == Mode::UN)
            UN_speed = speed_restriction(V_NVUNFIT, distance::from_odometer(dist_base::min), distance::from_odometer(dist_base::max), false);
        else
            UN_speed = {};
        if (mode == Mode::SH)
            SH_speed = speed_restriction(requested_mode_profile ? requested_mode_profile->speed : V_NVSHUNT, distance::from_odometer(dist_base::min), distance::from_odometer(dist_base::max), false);
        else
            SH_speed = {};
        if (mode == Mode::RV)
            RV_speed = speed_restriction(rv_supervision->speed, distance::from_odometer(dist_base::min), distance::from_odometer(dist_base::max), false);
        else
            RV_speed = {};

        if (mode == Mode::FS) {
            int64_t time = get_milliseconds();
            add_message(text_message(get_text("Entering FS"), true, false, false, [time](text_message &t){
                if (entering_mode_message_is_time_dependent) {
                    return time + 60000 < get_milliseconds();
                } else {
                    dist_base back = d_estfront_dir[odometer_orientation == -1] - L_TRAIN;
                    return get_gradient().empty() || SSP_begin() == dist_base::max || (SSP_begin() < back && get_gradient().begin()->first < back);
                }
            }));
        }
        if (mode == Mode::OS) {
            OS_speed = speed_restriction(requested_mode_profile ? requested_mode_profile->speed : V_NVONSIGHT, distance(std::numeric_limits<double>::lowest(), 0, 0), distance(std::numeric_limits<double>::max(), 0, 0), false);
            int64_t time = get_milliseconds();
            add_message(text_message(get_text("Entering OS"), true, false, false, [time](text_message &t){
                if (entering_mode_message_is_time_dependent) {
                    return time + 60000 < get_milliseconds();
                } else {
                    dist_base back = d_estfront_dir[odometer_orientation == -1] - L_TRAIN;
                    return get_gradient().empty() || SSP_begin() == dist_base::max || (SSP_begin() < back && get_gradient().begin()->first < back);
                }
            }));
        } else {
            OS_speed = {};
        }
        if (mode == Mode::TR) {
            train_trip(transition_index);
        }
        if (mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::UN || mode == Mode::SN)
            ongoing_mission = true;
        bool end_mission = false;
        if (mode == Mode::SB && (prevmode == Mode::FS || prevmode == Mode::LS || prevmode == Mode::OS || prevmode == Mode::UN || prevmode == Mode::NL || prevmode == Mode::SR || prevmode == Mode::PT || prevmode == Mode::RV || prevmode == Mode::SN))
            end_mission = true;
        if (mode == Mode::SH  && (prevmode == Mode::FS || prevmode == Mode::LS || prevmode == Mode::OS || prevmode == Mode::SR || prevmode == Mode::SN || prevmode == Mode::UN))
            end_mission = true;
        if (mode == Mode::SH && prevmode == Mode::PT && ongoing_mission)
            end_mission = true;
        if (end_mission) {
            ongoing_mission = false;
            for (auto *session : active_sessions) {
                if (!session->isRBC)
                    session->close();
                else if (session->status == session_status::Establishing)
                    session->finalize();
            }
            if (supervising_rbc) {
                auto *msg = new end_mission_message();
                supervising_rbc->queue(std::shared_ptr<euroradio_message_traintotrack>(msg));
            }
        }
        delete_information(prevmode);
        if (mode != Mode::NP && mode != Mode::PS && (mode != Mode::SH || prevmode != Mode::PS)) {
            if ((level == Level::N2 || level == Level::N3) && !end_mission
#if SIMRAIL
            && mode != Mode::SB // Out of SRS condition: do not start session to report mode change to SB
#endif
            && (!supervising_rbc || supervising_rbc->status == session_status::Inactive) && rbc_contact_valid) {
                set_supervising_rbc(*rbc_contact);
                    if (supervising_rbc)
                        supervising_rbc->open(N_tries_radio);
            }
            position_report_reasons[1] = true;
        }
        calculate_SvL();
        if (prevmode == Mode::NP) {
            initialize_cold_movement();
        }
    }
}
void set_mode_deleted_data()
{
    std::vector<std::vector<std::string>> infos = {
        {"U","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"D","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","U","U","U","NR","NR","U","D"},
        {"D","D","D","D","U","U","D","U","D","D","U","U","U","NR","NR","U","D"},
        {"D","D1,2","U","D2","U","U","D","U","D2","D1","D","U","U","NR","NR","D","D"},
        {"D","D","U","U","U","U","U","U","D","U","U","U","U","NR","NR","U","U"},
        {"D","D","U","U","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"D","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"D","D","D","D","D","D","U","D","D","D","D","D","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","U","U","D","D","U","U","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","U","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","U","U","D","D","U","U","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"R","R","R","R","U","U","U","U","R","R","U","U","U","NR","NR","R","U"},
        {"R","R","R","R","U","U","U","U","R","R","U","U","U","NR","NR","U","U"},
        {"D","D","D","D","U","U","U","U","D","D","U","U","U","NR","NR","D","U"},
        {"D","D","D","D","U","U","U","U","D","D","U","U","U","NR","NR","D","U"},
        {"D","U","D","D","U","U","U","U","D","U","U","U","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","D","D","D","D","D","U","NR","NR","D","D"},
        {"TBR","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"R","R","R","R","U","U","R","U","R","U","R","U","U","NR","NR","R","R"},
        {"R","R","R","R","U","U","R","R","R","R","R","R","R","NR","NR","R","R"},
        {"R","R","R","R","U","U","R","U","R","U","U","U","U","NR","NR","U","R"},
        {"D","D","D","D","U","U","D","U","D","D","D","U","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","U","U","NR","NR","D","D"},
        {"TBR","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"D","TBR","U","TBR","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"TBR","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"TBR","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"D","TBR","U","U","U","U","U","U","D","U","U","U","U","NR","NR","U","U"},
        {"U","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"TBR","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"D","TBR","U","U","U","U","U","U","D","U","U","U","U","NR","NR","U","U"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","U"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","U"},
        {"D","D","D","D","D","D","U","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","D","D","D","U","U","D","U","D","D","D","D","U","NR","NR","D","D"},
        {"D","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"U","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"U","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"U","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"U","U","U","U","U","U","U","U","U","U","U","U","U","NR","NR","U","U"},
        {"D","D","D","D","D","U","D","D","D","D","U","D","U","NR","NR","D","D"},
    };
    Mode modes[] = {Mode::NP, Mode::SB, Mode::PS, Mode::SH, Mode::FS, Mode::LS, Mode::SR, Mode::OS, Mode::SL,
        Mode::NL, Mode::UN, Mode::TR, Mode::PT, Mode::SF, Mode::IS, Mode::SN, Mode::RV};
    for (int i=0; i<infos.size(); i++) {
        for (int j=0; j<17; j++) {
            std::string s = infos[i][j];
            deleted_information info;
            info.index = i;
            info.deleted = !s.empty() && (s[0] == 'D' || s[0] == 'R');
            info.invalidated = s == "TBR";
            if (s == "D1" || s == "D1,2")
                info.exceptions.insert(1);
            if (s == "D2" || s == "D1,2")
                info.exceptions.insert(2);
            deleted_informations[modes[j]].push_back(info);
        }
    }
    information_list[2].delete_info = []() {delete_linking();};
    information_list[3].delete_info = []() {delete_MA();};
    information_list[4].delete_info = []() {delete_gradient();};
    information_list[5].delete_info = []() {delete_SSP();};
    information_list[6].delete_info = []() {delete_ASP();};
    information_list[7].delete_info = []() {STM_max_speed = {};};
    information_list[8].delete_info = []() {STM_system_speed = {};};
    information_list[9].delete_info = []() {ongoing_transition = {}; transition_buffer.clear(); sh_transition = {};};
    information_list[11].delete_info = []() {sh_balises = {};};
    information_list[12].delete_info = []() {ma_params = {30000, (int64_t)(T_CYCRQSTD*1000), 30000};};
    information_list[13].delete_info = []() {pos_report_params = {};};
    information_list[14].delete_info = []() {sr_balises = {};};
    information_list[16].delete_info = []() {inhibit_revocable_tsr = false;};
    information_list[17].delete_info = []() {default_gradient_tsr = 0;};
    information_list[18].delete_info = []() {signal_speeds.clear();};
    information_list[19].delete_info = []() {route_suitability.clear();};
    information_list[21].delete_info = []() {slippery_rail_driver=false;};
    information_list[22].delete_info = []() {
        for (auto &m : messages) {
            if (m.type == text_message_type::PlainText)
                m.end_condition = [](text_message&m){return true;};
        }
    };
    information_list[23].delete_info = []() {
        for (auto &m : messages) {
            if (m.type == text_message_type::FixedText)
                m.end_condition = [](text_message&m){return true;};
        }
    };
    information_list[25].delete_info = []() {reset_mode_profile(distance(), false);};
    information_list[29].delete_info = []() {
        for (auto it = track_conditions.begin(); it != track_conditions.end();) {
            TrackConditions c = it->get()->condition;
            if (c != TrackConditions::SoundHorn && c != TrackConditions::NonStoppingArea && c != TrackConditions::TunnelStoppingArea && c!= TrackConditions::BigMetalMasses)
                it = track_conditions.erase(it);
            else
                ++it;
        }
    };
    information_list[30].delete_info = []() {
        for (auto it = track_conditions.begin(); it != track_conditions.end();) {
            TrackConditions c = it->get()->condition;
            if (c == TrackConditions::SoundHorn || c == TrackConditions::NonStoppingArea || c == TrackConditions::TunnelStoppingArea)
                it = track_conditions.erase(it);
            else
                ++it;
        }
    };
    information_list[31].delete_info = []() {
        for (auto it = track_conditions.begin(); it != track_conditions.end();) {
            TrackConditions c = it->get()->condition;
            if (c == TrackConditions::BigMetalMasses)
                it = track_conditions.erase(it);
            else
                ++it;
        }
    };
    information_list[32].delete_info = []() {emergency_stops.clear();};
    information_list[33].delete_info = []() {emergency_stops.clear();};
    information_list[35].invalidate_info = []() {train_data_valid = false;};
    information_list[36].invalidate_info = []() {level_valid = false;};
    information_list[37].invalidate_info = []() {priority_levels_valid = false;};
    information_list[38].delete_info = []() {driver_id = "";};
    information_list[38].invalidate_info = []() {driver_id_valid = false;};
    information_list[41].invalidate_info = []() {train_running_number_valid = false;};
    information_list[41].delete_info = []() {train_running_number = 0;};
    information_list[42].delete_info = []() {rv_area = {};};
    information_list[43].delete_info = []() {rv_supervision = {};};
    information_list[44].delete_info = []() {taf_request = {};};
    information_list[45].delete_info = []() {PBDs.clear();};
    information_list[46].delete_info = []() {level_crossings.clear();};
    information_list[52].delete_info = []() {display_lssma_time = {};};
}
void trigger_condition(int num)
{
    for (mode_transition &t : ordered_transitions[(int)mode]) {
        if (t.conditions.find(num) != t.conditions.end()) {
            mode_conditions[num].trigger();
            break;
        }
    }
}
void delete_information(Mode prev)
{
    std::vector<deleted_information> info = deleted_informations[mode];
    for (int i=0; i<info.size(); i++) {
        info[i].handle(prev, mode);
    }
}