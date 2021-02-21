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
#include "mode_transition.h"
#include "train_trip.h"
#include "override.h"
#include "../Position/linking.h"
#include "../DMI/text_message.h"
#include "level_transition.h"
#include "../TrackConditions/track_condition.h"
#include <map>
cond mode_conditions[75];
static std::vector<mode_transition> transitions;
static std::vector<mode_transition> ordered_transitions[20];
Mode mode=Mode::SB;
bool mode_acknowledgeable=false;
bool mode_acknowledged=false;
Mode mode_to_ack;
void set_mode_deleted_data();
void initialize_mode_transitions()
{
    cond *c = mode_conditions;
    c[1] = [](){return false;};
    c[2] = [](){return true;};
    c[3] = [](){return V_est == 0;};
    c[4] = [](){return true;};
    c[7] = [](){return level!=Level::N0 && level!=Level::NTC && V_est==0 && mode_to_ack==Mode::TR && mode_acknowledged;};
    c[8] = [](){return mode_to_ack==Mode::SR && mode_acknowledged;};
    c[10] = [](){return train_data_valid() && MA && !get_SSP().empty() && !get_gradient().empty() && !requested_mode_profile;};
    c[12] = [](){return level == Level::N1 && EoA && *EoA<(d_minsafefront(EoA->get_reference())-L_antenna_front);};
    c[15] = [](){return mode_to_ack==Mode::OS && mode_acknowledged;};
    c[16] = [](){return (level == Level::N2 || level==Level::N3) && EoA && *EoA<d_minsafefront(EoA->get_reference());};
    c[21] = [](){return level == Level::N0;};
    c[25] = [](){return (level == Level::N1 || level == Level::N2 || level==Level::N3) && MA && !get_SSP().empty() && !get_gradient().empty() && !requested_mode_profile;};
    c[37] = [](){return false;};
    c[31] = [](){return MA && !get_SSP().empty() && !get_gradient().empty() && (level == Level::N2 || level==Level::N3) && !requested_mode_profile;};
    c[32] = [](){return MA && !get_SSP().empty() && !get_gradient().empty() && level == Level::N1 && MA->get_v_main() > 0 && !requested_mode_profile;};
    c[34] = [](){return !mode_profiles.empty() && mode_profiles.front().mode == Mode::OS && mode_profiles.front().start < d_maxsafefront(mode_profiles.front().start.get_reference())  && (level == Level::N1 || level == Level::N2 || level==Level::N3);};
    c[40] = [](){return !mode_profiles.empty() && mode_profiles.front().mode == Mode::OS && mode_profiles.front().start < d_maxsafefront(mode_profiles.front().start.get_reference());};
    c[42] = [](){return SR_dist && *SR_dist < d_estfront && !overrideProcedure;};
    c[43] = [](){return !overrideProcedure && formerEoA && *formerEoA<d_minsafefront(formerEoA->get_reference())-L_antenna_front;};
    c[39] = [](){return (level == Level::N1 || level == Level::N2 || level==Level::N3) && !MA;};
    c[50] = [](){return mode_to_ack==Mode::SH && mode_acknowledged;};
    c[51] = [](){return !mode_profiles.empty() && mode_profiles.front().mode == Mode::SH && mode_profiles.front().start < d_maxsafefront(mode_profiles.front().start.get_reference());};
    c[60] = [](){return mode_to_ack==Mode::UN && mode_acknowledged;};
    c[62] = [](){return (level==Level::N0 || level==Level::NTC) && V_est==0 && train_data_valid() && mode_acknowledged;;};
    c[68] = [](){return (level==Level::N0 || level==Level::NTC) && V_est==0 && !train_data_valid() && mode_acknowledged;;};
    c[69] = [](){return get_SSP().begin()->get_start()>d_estfront || get_gradient().begin()->first>d_estfront || (--get_gradient().end())->first<d_estfront || (--get_SSP().end())->get_end()<d_estfront;};
    c[70] = [](){return mode_to_ack==Mode::LS && mode_acknowledged;};
    c[73] = [](){return !(in_mode_ack_area && mode_to_ack == Mode::LS) && !mode_profiles.empty() && mode_profiles.front().mode == Mode::OS && mode_profiles.front().start < d_maxsafefront(mode_profiles.front().start.get_reference());};
    c[74] = [](){return !(in_mode_ack_area && mode_to_ack == Mode::OS) && !mode_profiles.empty() && mode_profiles.front().mode == Mode::LS && mode_profiles.front().start < d_maxsafefront(mode_profiles.front().start.get_reference());};

    transitions.push_back({Mode::SB, Mode::SR, {8,37}, 7});
    transitions.push_back({Mode::FS, Mode::SR, {37}, 6});
    transitions.push_back({Mode::LS, Mode::SR, {37}, 6});
    transitions.push_back({Mode::OS, Mode::SR, {37}, 6});
    transitions.push_back({Mode::PT, Mode::SR, {8,37}, 5});
    transitions.push_back({Mode::UN, Mode::SR, {44,45}, 4});
    transitions.push_back({Mode::SN, Mode::SR, {44,45}, 4});

    transitions.push_back({Mode::SB, Mode::FS, {29}, 7});
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
void update_mode_status()
{
    update_mode_profile();
    std::vector<mode_transition> &available=ordered_transitions[(int)mode];
    int priority = 10;
    int transition_index=-1;
    Mode transition = mode;
    for (mode_transition &t : available) {
        int i = t.happens();
        if (t.from == mode && i>=0 && t.priority < priority) {
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
        if (mode == Mode::TR || mode == Mode::LS || mode == Mode::OS || mode == Mode::SH)
            overrideProcedure = false;
        if (mode == Mode::SR) {
            if (std::isfinite(D_NVSTFF)) {
                SR_dist = distance(d_maxsafefront(0)+D_NVSTFF);
                SR_speed = speed_restriction(V_NVSTFF, distance(std::numeric_limits<double>::lowest()), *SR_dist, false);
            } else {
                SR_dist = {};
                SR_speed = speed_restriction(V_NVSTFF, distance(std::numeric_limits<double>::lowest()), distance(std::numeric_limits<double>::max()), false);
            }
        } else {
            SR_dist = {};
            SR_speed = {};
            formerEoA = {};
        }
        if (mode == Mode::UN)
            UN_speed = speed_restriction(V_NVUNFIT, distance(std::numeric_limits<double>::lowest()), distance(std::numeric_limits<double>::max()), false);
        else
            UN_speed = {};
        if (mode == Mode::SH)
            SH_speed = speed_restriction(requested_mode_profile ? requested_mode_profile->speed : V_NVSHUNT, distance(std::numeric_limits<double>::lowest()), distance(std::numeric_limits<double>::max()), false);
        else
            SH_speed = {};
        
        if (mode == Mode::FS) {
            add_message(text_message("Entering FS", true, false, false, [](text_message &t){
                distance back = d_estfront-L_TRAIN;
                return get_gradient().empty() || get_SSP().empty() || (get_SSP().begin()->get_start()<back && get_gradient().begin()->first<back);
            }));
        }
        if (mode == Mode::OS) {
            OS_speed = speed_restriction(requested_mode_profile ? requested_mode_profile->speed : V_NVONSIGHT, distance(std::numeric_limits<double>::lowest()), distance(std::numeric_limits<double>::max()), false);
            add_message(text_message("Entering OS", true, false, false, [](text_message &t){
                distance back = d_estfront-L_TRAIN;
                return get_gradient().empty() || get_SSP().empty() || (get_SSP().begin()->get_start()<back && get_gradient().begin()->first<back);
            }));
        } else {
            OS_speed = {};
        }
        if (mode == Mode::TR) {
            train_trip(transition_index);
        }
        std::vector<deleted_information> info = deleted_informations[mode];
        for (int i=0; i<info.size(); i++) {
            info[i].handle(prevmode, mode);
        }
        calculate_SvL();
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
            info.deleted = !s.empty() && s[0] == 'D';
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
    information_list[9].delete_info = []() {ongoing_transition = {}; transition_buffer.clear();};
    information_list[18].delete_info = []() {signal_speeds.clear();};
    information_list[25].delete_info = []() {reset_mode_profile(distance(), false);};
    information_list[29].delete_info = []() {
        for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
            TrackConditions c = it->get()->condition;
            if (c != TrackConditions::SoundHorn && c != TrackConditions::NonStoppingArea && c != TrackConditions::TunnelStoppingArea && c!= TrackConditions::BigMetalMasses) {
                auto next = it;
                ++next;
                track_conditions.erase(it);
                it = --next;
            }
        }
    };
    information_list[30].delete_info = []() {
        for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
            TrackConditions c = it->get()->condition;
            if (c == TrackConditions::SoundHorn || c == TrackConditions::NonStoppingArea || c == TrackConditions::TunnelStoppingArea) {
                auto next = it;
                ++next;
                track_conditions.erase(it);
                it = --next;
            }
        }
    };
    information_list[31].delete_info = []() {
        for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
            TrackConditions c = it->get()->condition;
            if (c == TrackConditions::BigMetalMasses) {
                auto next = it;
                ++next;
                track_conditions.erase(it);
                it = --next;
            }
        }
    };
    information_list[35].invalidate_info = []() {invalidate_train_data();};
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