/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "stm.h"
#include "../Supervision/speed_profile.h"
#include "../Procedures/override.h"
#include "../Packets/messages.h"
#include "../Packets/etcs_information.h"
#include "../Packets/logging.h"
#include "../Supervision/train_data.h"
#include "../Packets/STM/1.h"
#include "../Packets/STM/5.h"
#include "../Packets/STM/7.h"
#include "../Packets/STM/13.h"
#include "../Packets/STM/14.h"
#include "../Packets/STM/15.h"
#include "../Packets/STM/16.h"
#include "../Packets/STM/17.h"
#include "../Packets/STM/30.h"
#include "../Packets/STM/128.h"
#include "../Packets/STM/129.h"
#include "../Packets/STM/130.h"
#include "../Packets/STM/175.h"
#include "../Packets/STM/179.h"
#include "../Packets/STM/180.h"
#include "../Packets/STM/181.h"
#include "../Packets/STM/184.h"
#include "../language/language.h"
#include "../DMI/windows.h"
#include "../TrainSubsystems/train_interface.h"
#include "platform_runtime.h"
#include <orts/client.h>
std::map<int, stm_object*> installed_stms;
std::map<int, int> ntc_to_stm;
std::map<int, std::vector<int>> ntc_to_stm_lookup_table;
bool stm_control_EB = false;
bool ntc_unavailable_msg = false;
int STM_max_speed_ntc;
void sim_write_line(const std::string &str);
struct stm_transition
{
    stm_state from;
    stm_state to;
    std::set<std::string> conditions;
    std::string happens(stm_object *stm)
    {
        std::string cond = "";
        for (auto c : conditions) {
            if (stm->conditions[c]())
                cond = c;
        }
        return cond;
    }
    stm_transition(stm_state from, stm_state to, std::set<std::string> conditionnum) : from(from), to(to), conditions(conditionnum)
    {

    }
};
static std::map<int, std::vector<stm_transition>> ordered_transitions;
void stm_object::trigger_condition(std::string change) {
    auto &available = ordered_transitions[(int)state];
    for (auto &t : available) {
        if (t.conditions.find(change) != t.conditions.end()) {
            conditions[change].trigger();
            break;
        }
    }
}
void stm_object::request_state(stm_state req)
{
    if (state == req)
        return;
    if (req == stm_state::CO)
        trigger_condition("A2");
    else if (req == stm_state::DE && train_data_valid)
        trigger_condition("A3");
    else if (req == stm_state::CS)
        trigger_condition("A4a");
    else if (req == stm_state::FA && state != stm_state::FA)
        send_failed_msg(this);
    if (req == stm_state::CO && specific_data_need < 0)
        trigger_condition("L16");
    auto &available = ordered_transitions[(int)state];
    bool allowed = req == state;
    for (auto &t : available) {
        if (t.to == req)
            allowed = true;
    }
    if (!allowed)
        trigger_condition("A16");
}
stm_object::stm_object()
{
    tiu_function = stm_tiu_function();
    biu_function = stm_biu_function();
    state = stm_state::NP;
    last_order = {};
    last_order_time = 0;
    last_national_trip = 0;
    national_trip = isolated = false;
    data_entry = data_entry_state::Inactive;
    specific_data_need = -1;
    conditions["A1"] = [this] {return state == stm_state::PO;};
    conditions["E4a"] = [] {return mode == Mode::SB;};
    conditions["G4a"] = [this] {
        if (state == stm_state::HS && !overrideProcedure) {
            if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && get_stm(ongoing_transition->leveldata.nid_ntc) == this) {
                return false;
            }
            if (level == Level::NTC && get_stm(nid_ntc) == this) {
                return false;
            }
            return true;
        }
        return false;
    };
    conditions["H4a"] = []{return mode == Mode::SB && !cab_active[0] && !cab_active[1];};
    conditions["I4a"] = []{return mode == Mode::SH;};
    conditions["L4a"] = []{return mode == Mode::TR;};
    conditions["A6"] = [this]{
        if (state == stm_state::CS) {
            for (auto kvp : installed_stms) {
                auto *stm = kvp.second;
                if (stm != this && stm->state == stm_state::HS)
                    return false;
            }
            if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && get_stm(ongoing_transition->leveldata.nid_ntc) == this) {
                return true;
            }
        }
        return false;
    };
    conditions["B6"] = [this]{
        if (mode == Mode::SB && (cab_active[0]^cab_active[1]) && state == stm_state::CS && level == Level::NTC && level_valid) {
            for (auto kvp : installed_stms) {
                auto *stm = kvp.second;
                if (stm != this && stm->state == stm_state::HS)
                    return false;
            }
            if (get_stm(nid_ntc) == this)
                return true;
        }
        return false;
    };
    conditions["A9"] = [this] {
        if ((state == stm_state::CS || state == stm_state::HS) && level == Level::NTC && (mode == Mode::SN || mode == Mode::SL || mode == Mode::NL)) {
            for (auto kvp : installed_stms) {
                auto *stm = kvp.second;
                if (stm != this && stm->state == stm_state::DA)
                    return false;
            }
            if (get_stm(nid_ntc) == this)
                return true;
        }
        return false;
    };
    conditions["A15"] = [this] {return false;};
    conditions["C16"] = [this] {return last_order && *last_order != stm_state::DA && *last_order != stm_state::CCS && get_milliseconds() - last_order_time > 10000;};
    conditions["D16"] = [this] {return last_order && *last_order == stm_state::DA && get_milliseconds() - last_order_time > 5000;};
    conditions["E16"] = [this] {return last_order && *last_order == stm_state::CCS && !national_trip && get_milliseconds() - last_order_time > 5000;};
}
void fill_stm_transitions()
{
    std::vector<stm_transition> stm_transitions;
    stm_transitions.push_back({stm_state::NP, stm_state::PO, {"A1"}});
    stm_transitions.push_back({stm_state::CO, stm_state::PO, {"A1"}});
    stm_transitions.push_back({stm_state::DE, stm_state::PO, {"A1"}});
    stm_transitions.push_back({stm_state::CS, stm_state::PO, {"A1"}});
    stm_transitions.push_back({stm_state::HS, stm_state::PO, {"A1"}});
    stm_transitions.push_back({stm_state::DA, stm_state::PO, {"A1"}});
    stm_transitions.push_back({stm_state::FA, stm_state::PO, {"A1"}});
    stm_transitions.push_back({stm_state::PO, stm_state::CO, {"A2"}});
    stm_transitions.push_back({stm_state::CO, stm_state::DE, {"A3"}});
    stm_transitions.push_back({stm_state::CO, stm_state::CS, {"A4a"}});
    stm_transitions.push_back({stm_state::DE, stm_state::CS, {"A4a"}});
    stm_transitions.push_back({stm_state::CS, stm_state::HS, {"A6", "B6"}});
    stm_transitions.push_back({stm_state::CS, stm_state::DA, {"A9"}});
    stm_transitions.push_back({stm_state::HS, stm_state::DA, {"A9"}});
    stm_transitions.push_back({stm_state::NP, stm_state::FA, {"A17", "B16"}});
    stm_transitions.push_back({stm_state::PO, stm_state::FA, {"A16", "B16", "C16", "H16", "I16", "L16", "P16", "A17"}});
    stm_transitions.push_back({stm_state::CO, stm_state::FA, {"A16", "B16", "C16", "H16", "I16", "O16", "P16", "A17"}});
    stm_transitions.push_back({stm_state::DE, stm_state::FA, {"A16", "B16", "C16", "H16", "I16", "O16", "P16", "A17"}});
    stm_transitions.push_back({stm_state::CS, stm_state::FA, {"A16", "B16", "C16", "D16", "H16", "N16", "O16", "P16", "A17"}});
    stm_transitions.push_back({stm_state::HS, stm_state::FA, {"A16", "B16", "C16", "D16", "H16", "N16", "O16", "P16", "A17"}});
    stm_transitions.push_back({stm_state::DA, stm_state::FA, {"A16", "B16", "C16", "E16", "F16", "H16", "N16", "O16", "P16", "Q16", "A17"}});
    stm_transitions.push_back({stm_state::PO, stm_state::NP, {"A15"}});
    stm_transitions.push_back({stm_state::CO, stm_state::NP, {"A15"}});
    stm_transitions.push_back({stm_state::DE, stm_state::NP, {"A15"}});
    stm_transitions.push_back({stm_state::CS, stm_state::NP, {"A15"}});
    stm_transitions.push_back({stm_state::HS, stm_state::NP, {"A15"}});
    stm_transitions.push_back({stm_state::DA, stm_state::NP, {"A15"}});
    stm_transitions.push_back({stm_state::FA, stm_state::NP, {"A15"}});
    stm_transitions.push_back({stm_state::DA, stm_state::CCS, {"A4b","B4b"}});
    stm_transitions.push_back({stm_state::DA, stm_state::CS, {"B4a", "I4a", "E4a", "K4a", "L4a"}});
    stm_transitions.push_back({stm_state::HS, stm_state::CS, {"C4a", "E4b", "G4a", "H4b", "I4a", "J4a"}});

    for (auto &trans : stm_transitions) {
        ordered_transitions[(int)trans.from].push_back(trans);
    }
}
void update_ntc_transitions()
{
    for (auto kvp : installed_stms) {
        auto *stm = kvp.second;
        auto &available = ordered_transitions[(int)stm->state];
        for (stm_transition &t : available) {
            if (t.to != stm_state::FA && stm->last_order)
                continue;
            std::string type = t.happens(stm);
            if (type != "") {
#ifdef DEBUG_STM
                platform->debug_print("STM "+std::to_string(kvp.first)+": "+type);
#endif
                if (t.to != stm_state::NP && t.to != stm_state::PO && type != "A17") {
                    stm->last_order = t.to;
                    stm->last_order_time = get_milliseconds();
                    stm_message msg;
                    auto *order = new STMStateOrder();
                    order->NID_STMSTATEORDER.rawdata = (int)t.to;
                    msg.packets.push_back(std::shared_ptr<ETCS_packet>(order));
                    stm->send_message(msg);
                    if (t.to == stm_state::FA) {
                        stm->state = stm_state::FA;
                        send_failed_msg(stm);
                    }
                }
                if (t.to == stm_state::PO)
                    stm->state = stm_state::PO;
                break;
            }
        }
    }
}
void assign_stm(int nid_ntc, bool driver)
{
    if (ntc_to_stm.find(nid_ntc) == ntc_to_stm.end() || installed_stms.find(ntc_to_stm[nid_ntc]) == installed_stms.end() || !installed_stms[ntc_to_stm[nid_ntc]]->available()) {
        int nid_stm = -1;
        if (driver) {
            auto it = ntc_to_stm_lookup_table.find(nid_ntc);
            std::vector<int> table;
            if (it != ntc_to_stm_lookup_table.end())
                table = it->second;
            if (table.empty()) {
                nid_stm = nid_ntc;
            } else {
                for (auto nid : table) {
                    auto it = installed_stms.find(nid);
                    if (it != installed_stms.end() && it->second->available()) {
                        nid_stm = it->second->nid_stm;
                        break;
                    }
                }
                if (nid_stm < 0) {
                    nid_stm = table[0];
                }
                if (nid_stm < 0) {
                    nid_stm = table[0];
                }
            }
        } else {
            auto it = ntc_to_stm_lookup_table.find(nid_ntc);
            std::vector<int> table;
            if (it != ntc_to_stm_lookup_table.end())
                table = it->second;
            if (table.empty()) {
                nid_stm = nid_ntc;
            } else {
                for (auto nid : table) {
                    auto it = installed_stms.find(nid);
                    if (it != installed_stms.end() && it->second->available()) {
                        nid_stm = nid;
                        break;
                    }
                }
                if (nid_stm < 0) {
                    for (auto nid : table) {
                        auto it = installed_stms.find(nid);
                        if (it != installed_stms.end() && it->second->state != stm_state::FA && !it->second->isolated) {
                            nid_stm = nid;
                            break;
                        }
                    }
                }
                if (nid_stm < 0) {
                    nid_stm = table[0];
                }
            }
        }
        if (nid_stm >= 0)
            ntc_to_stm[nid_ntc] = nid_stm;
    }
}
void stm_level_change(level_information newlevel, bool driver)
{
    if (newlevel.level == Level::NTC && driver)
        assign_stm(newlevel.nid_ntc, true);
    if (level == Level::NTC) {
        auto *stm1 = get_stm(nid_ntc);
        if (stm1 != nullptr) {
            if (newlevel.level == Level::NTC) {
                auto *stm2 = get_stm(newlevel.nid_ntc);
                if (stm1 != stm2)
                    stm1->trigger_condition(driver ? "B4b" : "A4b");
            } else {
                stm1->trigger_condition(driver ? "K4a" : "B4a");
                if (stm1->national_trip)
                    mode_conditions[38].trigger();
            }
        }
    }
    if (newlevel.level == Level::NTC && !driver) {
        auto *stm = get_stm(newlevel.nid_ntc);
        if (stm != nullptr && !stm->available()) {
            stm->trigger_condition("I16");
        }
    }
    if (newlevel.level != Level::NTC || nid_ntc != newlevel.nid_ntc)
        ntc_to_stm.erase(nid_ntc);

    if (level == Level::NTC && nid_ntc == STM_max_speed_ntc && (newlevel.level != Level::NTC || newlevel.nid_ntc != STM_max_speed_ntc)) {
        STM_max_speed = {};
    }

    stm_message msg;
    auto *stat = new ETCSStatusData();
    stat->M_LEVEL.set_value(level);
    stat->NID_NTC.rawdata = nid_ntc;
    stat->M_MODESTM.set_value(mode);
    msg.packets.push_back(std::shared_ptr<ETCS_packet>(stat));
    stm_send_message(msg);
}
void stm_level_transition_received(level_transition_information info)
{
    if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC) {
        auto *stm1 = get_stm(ongoing_transition->leveldata.nid_ntc);
        if (stm1 != nullptr && stm1->state == stm_state::HS) {
            if (info.leveldata.level == Level::NTC) {
                auto stm2 = get_stm(info.leveldata.nid_ntc);
                if (stm1 != stm2) {
                    stm1->trigger_condition("C4a");
                }
            } else {
                stm1->trigger_condition("J4a");
            }
        }
    }
    if (!ongoing_transition || ongoing_transition->leveldata.level != Level::NTC || info.leveldata.level != Level::NTC || info.leveldata.nid_ntc != ongoing_transition->leveldata.nid_ntc) {
        STM_max_speed = {};
        STM_system_speed = {};
    }
}
void stm_object::report_trip()
{
    if (national_trip && (mode == Mode::PT || mode == Mode::UN))
        trigger_condition("Q16");
    last_national_trip = get_milliseconds();
}
void stm_object::report_override()
{
    if (active())
        start_override();
}
void stm_object::report_received(stm_state newstate)
{
    bool ordered = last_order && (*last_order == newstate || (*last_order == stm_state::CCS && newstate == stm_state::CS));
    if (state != newstate) {
        auto &available = ordered_transitions[(int)state];
        bool allowed = false;
        for (auto &t : available) {
            if (t.to == newstate)
                allowed = true;
        }
        if (newstate == stm_state::NP)
            trigger_condition("A15");
        else if (newstate == stm_state::FA)
            trigger_condition("A17");
        else if (!allowed || (!ordered && newstate != stm_state::PO))
            trigger_condition("B16");
    }
    if (state != newstate && newstate == stm_state::DA) {
        STM_max_speed = {};
        recalculate_MRSP();
    }
    if (state != newstate && newstate != stm_state::DA) {
        tiu_function = stm_tiu_function();
        biu_function = stm_biu_function();
    }
    state = newstate;
    if (ordered)
        last_order = {};
}
void stm_send_message(stm_message &msg, int nid_stm)
{
    msg.NID_STM.rawdata = nid_stm;
    bit_manipulator w;
    msg.write_to(w);
    sim_write_line("noretain(stm::command_etcs="+w.to_base64()+")");
    //log_message(*msg, d_estfront, get_milliseconds());
}
void stm_object::send_message(stm_message &msg)
{
    stm_send_message(msg, nid_stm);
}
void send_failed_msg(stm_object *stm)
{
    text_message msg(get_ntc_name(stm->nid_stm) + get_text(" failed"), true, true, 2, [stm](text_message &msg){return msg.acknowledged;});
    add_message(msg);
}
bool mode_filter(std::shared_ptr<etcs_information> info, const std::list<std::shared_ptr<etcs_information>> &message);
void request_STM_max_speed(stm_object *stm, double speed)
{
    if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && ongoing_transition->leveldata.nid_ntc != nid_ntc) {
        if (stm->state == stm_state::HS && stm == get_stm(ongoing_transition->leveldata.nid_ntc)) {
            auto info = std::shared_ptr<etcs_information>(new etcs_information(8));
            info->handle_fun = [speed]() {
                if (speed == -1) {
                    STM_max_speed = {};
                } else {
                    STM_max_speed = speed_restriction(speed, ongoing_transition->ref_loc ? *ongoing_transition->ref_loc + ongoing_transition->dist : distance::from_odometer(dist_base::min), distance::from_odometer(dist_base::max), false);
                    STM_max_speed_ntc = ongoing_transition->leveldata.nid_ntc;
                }
                recalculate_MRSP();
            };
            info->infill = {};
            info->timestamp = get_milliseconds();
            std::list<std::shared_ptr<etcs_information>> msg = {info};
            if (mode_filter(info, msg))
                info->handle();
        }
    }
}
void request_STM_system_speed(stm_object *stm, double speed, double dist)
{
    if (ongoing_transition && !ongoing_transition->immediate && ongoing_transition->leveldata.level == Level::NTC && level != Level::NTC) {
        auto it = ntc_to_stm.find(ongoing_transition->leveldata.nid_ntc);
        if (stm->state == stm_state::HS && stm == get_stm(ongoing_transition->leveldata.nid_ntc)) {
            auto info = std::shared_ptr<etcs_information>(new etcs_information(9));
            distance start = *ongoing_transition->ref_loc + ongoing_transition->dist - dist;
            distance end = *ongoing_transition->ref_loc + ongoing_transition->dist;
            info->handle_fun = [speed, start, end]() {
                if (speed == -1)
                    STM_system_speed = {};
                else
                    STM_system_speed = speed_restriction(speed, start, end, false);
                recalculate_MRSP();
            };
            info->infill = {};
            info->timestamp = get_milliseconds();
            std::list<std::shared_ptr<etcs_information>> msg = {info};
            if (mode_filter(info, msg))
                info->handle();
        }
    }
}
stm_object *get_stm(int nid_ntc)
{
    auto stm_it = ntc_to_stm.find(nid_ntc);
    if (stm_it == ntc_to_stm.end() || installed_stms.find(stm_it->second) == installed_stms.end())
        return nullptr;
    return installed_stms[stm_it->second];
}
std::string get_ntc_name(int nid_ntc)
{
    auto it = ntc_names.find(nid_ntc);
    if (it != ntc_names.end())
        return it->second;
    return "NTC "+std::to_string(nid_ntc);
}
static Mode prev_mode;
static bool prev_override;
void update_stm_control()
{
    if (level != Level::NTC)
        nid_ntc = -1;
    if (mode != prev_mode) {
        if(mode == Mode::NP/* || mode == Mode::SB*/)
            ntc_to_stm.clear();
        stm_message msg;
        auto *stat = new ETCSStatusData();
        stat->M_LEVEL.set_value(level);
        stat->NID_NTC.rawdata = nid_ntc;
        stat->M_MODESTM.set_value(mode);
        msg.packets.push_back(std::shared_ptr<ETCS_packet>(stat));
        stm_send_message(msg);
    }
    prev_mode = mode;
    if (prev_override != overrideProcedure) {
        stm_message msg;
        auto *ov = new STMOverrideStatus();
        ov->Q_OVR_STATUS.rawdata = overrideProcedure;
        msg.packets.push_back(std::shared_ptr<ETCS_packet>(ov));
        stm_send_message(msg);
    }
    prev_override = overrideProcedure;

    for (auto kvp : installed_stms) {
        auto *stm = kvp.second;
        stm->national_trip = get_milliseconds() - kvp.second->last_national_trip < 10000;
		bool entry_timer = (stm->data_entry == stm_object::data_entry_state::Start || stm->data_entry == stm_object::data_entry_state::DataSent)
            && get_milliseconds() - stm->data_entry_timer > 10000;
		if (entry_timer || (stm->data_entry != stm_object::data_entry_state::Inactive && active_dialog != dialog_sequence::NTCData)) {
			if (entry_timer)
				stm->trigger_condition("O16");
			stm->data_entry = stm_object::data_entry_state::Inactive;
			stm_message msg;
			auto *flag = new STMDataEntryFlag();
			msg.packets.push_back(std::shared_ptr<ETCS_packet>(flag));
			flag->M_DATAENTRYFLAG.rawdata = M_DATAENTRYFLAG_t::Stop;
			stm->send_message(msg);
		}
	}

    update_ntc_transitions();

    auto *stm = get_stm(nid_ntc);
    bool msg = false;
    if (stm != nullptr && !stm->available() && !stm->isolated) {
        stm->control_request_EB = true;
        if (mode == Mode::SN || (mode == Mode::NL && get_milliseconds()-last_mode_change > 5000)) {
            if (!ntc_unavailable_msg) {
                ntc_unavailable_msg = true;
                text_message msg(get_ntc_name(nid_ntc) + get_text(" not available"), true, false, 2, [stm](text_message &msg){
                    if (stm != get_stm(nid_ntc) || stm->available() || stm->isolated || (mode != Mode::SN && mode != Mode::NL)) {
                        ntc_unavailable_msg = false;
                        return true;
                    }
                    return false;
                });
                add_message(msg);
            }
        }
    }
    stm_control_EB = false;
    for (auto kvp : installed_stms) {
        auto *stm2 = kvp.second;
        if (stm2->state == stm_state::DA || level == Level::N0 || level == Level::N1 || level == Level::N2 || level == Level::N3 || stm != stm2 || mode != Mode::SN || stm->isolated)
            stm2->control_request_EB = false;
        if (stm2->last_order && *stm2->last_order == stm_state::CCS && stm2->national_trip && stm2->state != stm_state::CS && (stm2->state != stm_state::FA || V_est > 0))
            stm2->control_request_EB = true;
        stm_control_EB |= stm2->control_request_EB;
    }
    if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && (!STM_max_speed || STM_max_speed->get_speed() > 0)) {
        stm = get_stm(ongoing_transition->leveldata.nid_ntc);
        if (stm != nullptr && !stm->available())
            request_STM_max_speed(stm, 0);
    }
    if (!ongoing_transition)
        STM_system_speed = {};
    if (level == Level::NTC && nid_ntc == STM_max_speed_ntc) {
        stm = get_stm(nid_ntc);
        if (stm == nullptr || stm->state == stm_state::DA || stm->state == stm_state::FA)
            STM_max_speed = {};
    }
}
void stm_send_train_data()
{
    for (auto &kvp : installed_stms) {
        auto *stm = kvp.second;
        if (stm->state == stm_state::CO || stm->state == stm_state::DE || stm->state == stm_state::CS || stm->state == stm_state::HS || stm->state == stm_state::DA) {
            stm_message msg;
            if (stm->specific_data_need > 0) {
                stm->specific_data.clear();
                stm->data_entry = stm_object::data_entry_state::Start;
                stm->data_entry_timer = get_milliseconds();
                auto *flag = new STMDataEntryFlag();
                flag->M_DATAENTRYFLAG.rawdata = M_DATAENTRYFLAG_t::Start;
                msg.packets.push_back(std::shared_ptr<ETCS_packet>(flag));
            }
            auto *td = new STMTrainData();
            td->NC_CDTRAIN.set_value(cant_deficiency);
            td->NC_TRAIN.rawdata = 0;
            for (int t : other_train_categories) {
                td->NC_TRAIN.rawdata |= 1<<t;
            }
            td->L_TRAIN.set_value(L_TRAIN);
            td->V_MAXTRAIN.set_value(V_train);
            td->M_AIRTIGHT.rawdata = Q_airtight ? td->M_AIRTIGHT.Fitted : td->M_AIRTIGHT.NotFitted;
            msg.packets.push_back(std::shared_ptr<ETCS_packet>(td));
            stm->send_message(msg);
        }
    }
}
void stm_object::send_specific_data(json &result)
{
    data_entry = stm_object::data_entry_state::DataSent;
    data_entry_timer = get_milliseconds();
    stm_message msg;
    auto *res = new STMSpecificDataEntryValues();
    std::map<int, std::string> vals;
    for (auto it = result.begin(); it!=result.end(); ++it) {
        for (auto &field : specific_data) {
            if (field.caption == it.key())
                vals[field.id] = it.value();
        }
    }
    res->N_ITER.rawdata = vals.size();
    for (auto &kvp : vals) {
        STMDataFieldResult dfr;
        dfr.NID_DATA.rawdata = kvp.first;
        dfr.L_VALUE.rawdata = kvp.second.size();
        for (int i=0; i<kvp.second.size(); i++) {
            X_VALUE_t val;
            val.rawdata = kvp.second[i];
            dfr.X_VALUE.push_back(val);
        }
        res->results.push_back(dfr);
    }
    msg.packets.push_back(std::shared_ptr<ETCS_packet>(res));
    send_message(msg);
    specific_data.clear();
}
void setup_stm_control()
{
    ntc_names[0] = "ASFA";
    ntc_names[1] = "ATB";
    ntc_names[2] = "ASFA AVE";
    ntc_names[3] = "LZB (C)";
    ntc_names[5] = "TBL 1";
    ntc_names[6] = "PZB 90";
    ntc_names[7] = "TBL 2/3";
    ntc_names[8] = "KVB";
    ntc_names[9] = "LZB";
    ntc_names[10] = "LZB (E)";
    ntc_names[11] = "SCMT";
    ntc_names[12] = "MEMOR II+";
    ntc_names[14] = "TVM";
    ntc_names[15] = "BACC";
    ntc_names[16] = "RSDD";
    ntc_names[17] = "EVM";
    ntc_names[18] = "Crocodile";
    ntc_names[19] = "EBICAB 900";
    ntc_names[20] = "TPWS/AWS";
    ntc_names[21] = "TPWS/AWS (SA)";
    ntc_names[22] = "ATC2";
    ntc_names[23] = "EBICAB 900";
    ntc_names[24] = "EBICAB 900 (PL)";
    ntc_names[25] = "KNR ATS";
    ntc_names[26] = "SHP";
    ntc_names[27] = "INDUSI I 60";
    ntc_names[28] = "TBL1+";
    ntc_names[29] = "NExTEO";
    ntc_names[30] = "ZUB 123";
    ntc_names[32] = "RPS";
    ntc_names[33] = "LS";
    ntc_names[34] = "EBICAB 700 (P)";
    ntc_names[35] = "SELCAB";
    ntc_names[36] = "INDUSI I 60";
    ntc_names[37] = "TBL";
    ntc_names[39] = "ALSN";
    ntc_names[40] = "EBICAB";
    ntc_names[45] = "CTCS-2";
    ntc_names[46] = "EBICAB 700";
    ntc_names[50] = "TGMT";
    ntc_to_stm_lookup_table[0] = {0, 19};
    fill_stm_transitions();
}
void handle_stm_message(stm_message &msg)
{
    int nid_stm = msg.NID_STM;
    if (installed_stms.find(nid_stm) == installed_stms.end()) {
        installed_stms[nid_stm] = new stm_object();
        installed_stms[nid_stm]->nid_stm = nid_stm;
    }
    //log_message(msg, d_estfront, get_milliseconds());
    stm_object *stm = installed_stms[nid_stm];
    for (auto &pack : msg.packets) {
        switch((unsigned char)pack->NID_PACKET.rawdata) {
            case 1:
            {
                auto &ver = *((STMVersion*)pack.get());
                if (ver.N_VERMAJOR != 4) break;
                stm_message msg;
                msg.packets.push_back(pack);
                auto *lang = new STMLanguage();
                lang->NID_DRV_LANGUAGE.rawdata = ((language[0]&0xFF)<<8)|(language[1]&0xFF);
                msg.packets.push_back(std::shared_ptr<ETCS_packet>(lang));
                auto *stat = new ETCSStatusData();
                stat->M_LEVEL.set_value(level);
                stat->NID_NTC.rawdata = nid_ntc;
                stat->M_MODESTM.set_value(mode);
                msg.packets.push_back(std::shared_ptr<ETCS_packet>(stat));
                stm->send_message(msg);
                break;
            }
            case 6:
                stm->report_override();
                break;
            case 13:
                stm->request_state((stm_state)((STMStateRequest*)pack.get())->NID_STMSTATEREQUEST.rawdata);
                break;
            case 15:
                stm->report_received((stm_state)((STMStateReport*)pack.get())->NID_STMSTATE.rawdata);
                break;
            case 16:
            {
                auto &max = *((STMMaxSpeed*)pack.get());
                if (level != Level::NTC || stm != get_stm(nid_ntc)) {
                    if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && stm == get_stm(ongoing_transition->leveldata.nid_ntc))
                        request_STM_max_speed(stm, max.V_STMMAX == V_STMMAX_t::NoMaxSpeed ? -1 : max.V_STMMAX.get_value());
                }
                break;
            }
            case 17:
            {
                auto &sys = *((STMSystemSpeed*)pack.get());
                if (level != Level::NTC) {
                    if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && stm == get_stm(ongoing_transition->leveldata.nid_ntc))
                        request_STM_system_speed(stm, sys.V_STMSYS == V_STMSYS_t::NoSystemSpeed ? -1 : sys.V_STMSYS.get_value(), sys.D_STMSYS.get_value());
                }
                break;
            }
            case 18:
                stm->report_trip();
                break;
            case 128:
            {
                auto &emerg = *((STMBrakeCommand*)pack.get());
                auto &biu = stm->biu_function;
                if (emerg.M_BIEB_CMD != M_BIEB_CMD_t::NoChange) {
                    biu.EB = emerg.M_BIEB_CMD != M_BIEB_CMD_t::ReleaseEB;
                }
                if (emerg.M_BISB_CMD != M_BISB_CMD_t::NoChange) {
                    biu.SB = emerg.M_BISB_CMD != M_BISB_CMD_t::ReleaseSB;
                    biu.EB_on_SB_failure = emerg.M_BISB_CMD == M_BISB_CMD_t::ApplySBorEB;
                }
                break;
            }
            case 129:
            {
                auto &ti = *((STMSpecificBrakeCommand*)pack.get());
                auto &tiu = stm->tiu_function;
                if (ti.M_TIRB_CMD == M_TIRB_CMD_t::AllowRegenerative)
                    tiu.regenerative_brake_inhibition = false;
                else if (ti.M_TIRB_CMD == M_TIRB_CMD_t::SupressRegenerative)
                    tiu.regenerative_brake_inhibition = true;
                if (ti.M_TIMSH_CMD == M_TIMSH_CMD_t::AllowMagnetic)
                    tiu.magnetic_shoe_inhibition = false;
                else if (ti.M_TIMSH_CMD == M_TIMSH_CMD_t::SupressMagnetic)
                    tiu.magnetic_shoe_inhibition = true;
                if (ti.M_TIEDCBEB_CMD == M_TIEDCBEB_CMD_t::AllowEddyEB)
                    tiu.eddy_emergency_brake_inhibition = false;
                else if (ti.M_TIEDCBEB_CMD == M_TIEDCBEB_CMD_t::SupressEddyEB)
                    tiu.eddy_emergency_brake_inhibition = true;
                if (ti.M_TIEDCBSB_CMD == M_TIEDCBSB_CMD_t::AllowEddySB)
                    tiu.eddy_service_brake_inhibition = false;
                else if (ti.M_TIEDCBSB_CMD == M_TIEDCBSB_CMD_t::SupressEddySB)
                    tiu.eddy_service_brake_inhibition = true;
                break;
            }
            case 130:
            {
                auto &ti = *((STMTrainCommand*)pack.get());
                auto &tiu = stm->tiu_function;
                if (ti.M_TIPANTO_CMD == M_TIPANTO_CMD_t::PantoLift)
                    tiu.lower_pantograph = false;
                else if (ti.M_TIPANTO_CMD == M_TIPANTO_CMD_t::PantoLower)
                    tiu.lower_pantograph = true;
                if (ti.M_TIFLAP_CMD == M_TIFLAP_CMD_t::FlatClose)
                    tiu.close_air_intake = true;
                else if (ti.M_TIFLAP_CMD == M_TIFLAP_CMD_t::FlatOpen)
                    tiu.close_air_intake = true;
                if (ti.M_TIMS_CMD == M_TIMS_CMD_t::MainSwitchOpen)
                    tiu.open_circuit_breaker = true;
                else if (ti.M_TIMS_CMD == M_TIMS_CMD_t::MainSwitchClose)
                    tiu.open_circuit_breaker = false;
                if (ti.M_TITR_CMD == M_TITR_CMD_t::TCO)
                    tiu.TCO = true;
                else if (ti.M_TITR_CMD == M_TITR_CMD_t::NoTCO)
                    tiu.TCO = false;
                break;
            }
            case 179:
            {
                auto &specific = *((STMSpecificDataEntryRequest*)pack.get());
                if (specific.N_ITER.rawdata != 0) {
                    for (auto &field : specific.fields) {
                        stm->specific_data.push_back(stm_specific_data(field));
                    }
                    if (specific.Q_FOLLOWING == Q_FOLLOWING_t::NoFollowing)
                        stm->data_entry = stm_object::data_entry_state::Active;
                    stm->data_entry_timer = get_milliseconds();
                } else {
                    stm->data_entry = stm_object::data_entry_state::Inactive;
                    stm_message msg;
                    auto *flag = new STMDataEntryFlag();
                    msg.packets.push_back(std::shared_ptr<ETCS_packet>(flag));
                    flag->M_DATAENTRYFLAG.rawdata = M_DATAENTRYFLAG_t::Stop;
                    stm->send_message(msg);
                }
                break;
            }
            case 181:
                stm->specific_data_need = ((STMSpecificDataNeed*)pack.get())->Q_DATAENTRY.rawdata == Q_DATAENTRY_t::SpecificDataNeeded;
                break;
        }
    }
}
