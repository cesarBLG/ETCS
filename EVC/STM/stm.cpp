#include "stm.h"
#include "../Supervision/speed_profile.h"
#include "../Procedures/override.h"
#include "../Packets/messages.h"
#include "../Packets/etcs_information.h"
#include "../Supervision/train_data.h"
#include "../Packets/STM/13.h"
#include "../Packets/STM/14.h"
#include "../Packets/STM/15.h"
#include "../Packets/STM/128.h"
#include "../Packets/STM/175.h"
#include "../Packets/STM/181.h"
#include "../Packets/STM/184.h"
#include <orts/client.h>
std::map<int, stm_object*> installed_stms;
std::map<int, int> ntc_to_stm;
std::map<int, std::vector<stm_object*>> ntc_to_stm_lookup_table;
bool stm_control_EB = false;
bool ntc_unavailable_msg = false;
extern ORserver::POSIXclient *s_client;
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
    if (req == stm_state::CO && specific_data < 0)
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
    commands = stm_commands();
    state = stm_state::NP;
    last_order = {};
    last_order_time = 0;
    last_national_trip = 0;
    national_trip = isolated = false;
    specific_data = -1;
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
    conditions["H4a"] = []{return mode == Mode::SB && !desk_open;};
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
        if (mode == Mode::SB && desk_open && state == stm_state::CS && level == Level::NTC && level_valid) {
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
    conditions["C16"] = [this] {return last_order && *last_order != stm_state::DA && *last_order != stm_state::CCS && get_milliseconds() - last_order_time > 10000;};
    conditions["D16"] = [this] {return last_order && *last_order == stm_state::DA && get_milliseconds() - last_order_time > 5000;};
    conditions["E16"] = [this] {return last_order && *last_order == stm_state::CCS && !national_trip && get_milliseconds() - last_order_time > 5000;};
    conditions["O16"] = [this] {return (data_entry == data_entry_state::Start || data_entry == data_entry_state::DataSent) && get_milliseconds() - data_entry_timer > 10000;};
}
void fill_stm_transitions()
{
    std::vector<stm_transition> stm_transitions;
    stm_transitions.push_back({stm_state::NP, stm_state::PO, {"A1"}});
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
    stm_transitions.push_back({stm_state::FA, stm_state::NP, {"A15"}});
    stm_transitions.push_back({stm_state::FA, stm_state::PO, {"A1"}});
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
                if (t.to != stm_state::NP && t.to != stm_state::PO && type != "A17") {
                    stm->last_order = t.to;
                    stm->last_order_time = get_milliseconds();
                    std::cout<<"STM: "<<type<<std::endl;
                    
                    auto *msg = new stm_message();
                    msg->NID_STM.rawdata = kvp.first;
                    auto *order = new STMStateOrder();
                    order->NID_STMSTATEORDER.rawdata = (int)t.to;
                    msg->packets.push_back(std::shared_ptr<ETCS_packet>(order));
                    bit_manipulator w;
                    msg->write_to(w);
                    s_client->WriteLine("noretain(stm::command_etcs="+w.to_base64()+")");
                    
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
    if (ntc_to_stm.find(nid_ntc) == ntc_to_stm.end()) {
        int nid_stm = -1;
        if (driver) {
            auto it = ntc_to_stm_lookup_table.find(nid_ntc);
            std::vector<stm_object*> table;
            if (it != ntc_to_stm_lookup_table.end())
                table = it->second;
            if (table.empty()) {
                nid_stm = nid_ntc;
            } else {
                for (auto *stm : table) {
                    if (stm->available()) {
                        nid_stm = stm->nid_stm;
                        break;
                    }
                }
                if (nid_stm < 0) {
                    nid_stm = (*table.begin())->nid_stm;
                }
                if (nid_stm < 0) {
                    nid_stm = (*table.begin())->nid_stm;
                }
            }
        } else {
            auto it = ntc_to_stm_lookup_table.find(nid_ntc);
            std::vector<stm_object*> table;
            if (it != ntc_to_stm_lookup_table.end())
                table = it->second;
            if (table.empty()) {
                nid_stm = nid_ntc;
            } else {
                for (auto *stm : table) {
                    if (stm->available()) {
                        nid_stm = stm->nid_stm;
                        break;
                    }
                }
                if (nid_stm < 0) {
                    for (auto *stm : table) {
                        if (stm->state != stm_state::FA && !stm->isolated) {
                            nid_stm = stm->nid_stm;
                            break;
                        }
                    }
                }
                if (nid_stm < 0) {
                    nid_stm = (*table.begin())->nid_stm;
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
}
void stm_level_transition_received(level_transition_information info)
{
    if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC) {
        auto *stm1 = get_stm(ongoing_transition->leveldata.nid_ntc);
        if (stm1->state == stm_state::HS) {
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
    if (state != newstate) {
        auto &available = ordered_transitions[(int)state];
        bool allowed = false;
        for (auto &t : available) {
            if (t.to == newstate)
                allowed = true;
        }
        if (!allowed)
            trigger_condition("B16");
        if (newstate == stm_state::FA)
            trigger_condition("A17");
    }

    if (state != newstate && newstate == stm_state::DA) {
        STM_max_speed = {};
        recalculate_MRSP();
    }
    state = newstate;
    if (last_order && (*last_order == state || (*last_order == stm_state::CCS && state == stm_state::CS)))
        last_order = {};

}
void send_failed_msg(stm_object *stm)
{
    text_message msg(get_ntc_name(stm->nid_stm) + " failed", true, true, 2, [stm](text_message &msg){return msg.acknowledged;});
    add_message(msg);
}
bool mode_filter(std::shared_ptr<etcs_information> info, std::list<std::shared_ptr<etcs_information>> message);
void request_STM_max_speed(stm_object *stm, double speed)
{
    if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && ongoing_transition->leveldata.nid_ntc != nid_ntc) {
        if (stm->state == stm_state::HS && stm == get_stm(ongoing_transition->leveldata.nid_ntc)) {
            auto info = std::shared_ptr<etcs_information>(new etcs_information(8));
            info->handle_fun = [speed]() {
                STM_max_speed = speed_restriction(speed, ongoing_transition->start, distance(std::numeric_limits<double>::max(), 0, 0), false);
                recalculate_MRSP();
            };
            std::list<std::shared_ptr<etcs_information>> msg = {info};
            if (mode_filter(info, msg))
                info->handle();
        }
    }
}
void request_STM_system_speed(stm_object *stm, double speed, double dist)
{
    if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && level != Level::NTC) {
        auto it = ntc_to_stm.find(ongoing_transition->leveldata.nid_ntc);
        if (stm->state == stm_state::HS && stm == get_stm(ongoing_transition->leveldata.nid_ntc)) {
            auto info = std::shared_ptr<etcs_information>(new etcs_information(9));
            distance start = ongoing_transition->start - dist;
            distance end = ongoing_transition->start;
            info->handle_fun = [speed, start, end]() {
                STM_system_speed = speed_restriction(speed, start, end, false);
                recalculate_MRSP();
            };
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
void update_stm_control()
{
    if (level != Level::NTC)
        nid_ntc = -1;
    if (mode != prev_mode && (mode == Mode::NP || mode == Mode::SB))
        ntc_to_stm.clear();
    prev_mode = mode;

    for (auto kvp : installed_stms) {
        kvp.second->national_trip = get_milliseconds() - kvp.second->last_national_trip < 10000;
    }

    update_ntc_transitions();

    auto *stm = get_stm(nid_ntc);
    bool msg = false;
    if (stm != nullptr && !stm->available() && !stm->isolated) {
        stm->control_request_EB = true;
        if (mode == Mode::SN || (mode == Mode::NL && get_milliseconds()-last_mode_change > 5000)) {
            if (!ntc_unavailable_msg) {
                ntc_unavailable_msg = true;
                text_message msg(get_ntc_name(nid_ntc) + " not available", true, false, 2, [stm](text_message &msg){
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
        if (stm2->last_order && *stm2->last_order == stm_state::CCS && stm2->state != stm_state::CS && (stm2->state != stm_state::FA || V_est > 0))
            stm_control_EB = true;
        stm_control_EB |= stm2->control_request_EB;
    }
    if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && !STM_max_speed) {
        stm = get_stm(ongoing_transition->leveldata.nid_ntc);
        if (stm != nullptr && !stm->available())
            request_STM_max_speed(stm, 0);
    }
}
void stm_send_train_data()
{
    for (auto &kvp : installed_stms) {
        auto *stm = kvp.second;
        if (stm->state == stm_state::CO || stm->state == stm_state::DE || stm->state == stm_state::CS || stm->state == stm_state::HS || stm->state == stm_state::DA) {
            stm->data_entry = stm_object::data_entry_state::Start;
            auto *msg = new stm_message();
            msg->NID_STM.rawdata = kvp.first;
            auto *flag = new STMDataEntryFlag();
            msg->packets.push_back(std::shared_ptr<ETCS_packet>(flag));
            flag->M_DATAENTRYFLAG.rawdata = M_DATAENTRYFLAG_t::Start;
            auto *td = new STMTrainData();
            td->NC_CDTRAIN.set_value(cant_deficiency);
            td->NC_TRAIN.rawdata = 0;
            for (int t : other_train_categories) {
                td->NC_TRAIN.rawdata |= 1<<t;
            }
            td->L_TRAIN.set_value(L_TRAIN);
            td->V_MAXTRAIN.set_value(V_train);
            td->M_AIRTIGHT.rawdata = Q_airtight ? M_AIRTIGHT_t::Fitted : M_AIRTIGHT_t::NotFitted;
            msg->packets.push_back(std::shared_ptr<ETCS_packet>(td));
            bit_manipulator w;
            msg->write_to(w);
            s_client->WriteLine("noretain(stm::command_etcs="+w.to_base64()+")");
            stm->data_entry_timer = get_milliseconds();
        }
    }
}
void setup_stm_control()
{
    ntc_names[0] = "ASFA";
    ntc_names[10] = "LZB";
    fill_stm_transitions();
}
void handle_stm_message(const stm_message &msg)
{
    int nid_stm = msg.NID_STM;
    if (installed_stms.find(nid_stm) == installed_stms.end()) {
        installed_stms[nid_stm] = new stm_object();
        installed_stms[nid_stm]->nid_stm = nid_stm;
    }
    stm_object *stm = installed_stms[nid_stm];
    for (auto &pack : msg.packets) {
        switch((unsigned char)pack->NID_PACKET.rawdata) {
            case 6:
                stm->report_override();
                break;
            case 13:
                stm->request_state((stm_state)((STMStateRequest*)pack.get())->NID_STMSTATEREQUEST.rawdata);
                break;
            case 15:
                stm->report_received((stm_state)((STMStateReport*)pack.get())->NID_STMSTATE.rawdata);
                break;
            case 18:
                stm->report_trip();
                break;
            case 128:
            {
                auto &emerg = *((STMBrakeCommand*)pack.get());
                if (emerg.M_BIEB_CMD != M_BIEB_CMD_t::NoChange) {
                    stm->commands.EB = emerg.M_BIEB_CMD != M_BIEB_CMD_t::ReleaseEB;
                }
                if (emerg.M_BISB_CMD != M_BISB_CMD_t::NoChange) {
                    stm->commands.SB = emerg.M_BISB_CMD != M_BISB_CMD_t::ReleaseSB;
                    stm->commands.EB_on_SB_failure = emerg.M_BISB_CMD == M_BISB_CMD_t::ApplySBorEB;
                }
                break;
            }
            case 179:
            {
                stm->data_entry = stm_object::data_entry_state::Inactive;
                stm->data_entry_timer = get_milliseconds();
                auto *msg = new stm_message();
                msg->NID_STM.rawdata = nid_stm;
                auto *flag = new STMDataEntryFlag();
                msg->packets.push_back(std::shared_ptr<ETCS_packet>(flag));
                flag->M_DATAENTRYFLAG.rawdata = M_DATAENTRYFLAG_t::Stop;
                bit_manipulator w;
                msg->write_to(w);
                s_client->WriteLine("noretain(stm::command_etcs="+w.to_base64()+")");
                break;
            }
            case 181:
                stm->specific_data = ((STMSpecificDataNeed*)pack.get())->Q_DATAENTRY.rawdata == Q_DATAENTRY_t::SpecificDataNeeded;
                break;
        }
    }
}