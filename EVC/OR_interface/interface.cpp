/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "interface.h"
#include <set>
#include <bitset>
#include <algorithm>
#include "../Position/distance.h"
#include "../Supervision/speed_profile.h"
#include "../Supervision/supervision_targets.h"
#include "../MA/movement_authority.h"
#include "../Position/linking.h"
#include "../Packets/messages.h"
#include "../TrainSubsystems/power.h"
#include "../TrainSubsystems/train_interface.h"
#include "../STM/stm.h"
#include "../../DMI/time_etcs.h"
#include "../Config/config.h"
#include <orts/common.h>
#include "platform_runtime.h"
#include "orts_wrapper.h"
#include "../language/language.h"

//using namespace ORserver;

using std::string;
extern double V_est;
double V_set;
extern distance d_estfront;
extern bool EB_command;
extern bool SB_command;
extern bool desk_open;
double or_dist;
int ack_button_light;
int TimeOffset::offset;
ORserver::ParameterManager manager;

//std::list<euroradio_message_traintotrack> pendingmessages;
void parse_command(string str);
void SetParameters()
{
    ORserver::Parameter *p = new ORserver::Parameter("distance");
    p->SetValue = [](string val) {
        or_dist = stod(val);
        if (odometer_value > or_dist)
            odometer_direction = -1;
        else if (odometer_value < or_dist)
            odometer_direction = 1;
        
        odometer_value = or_dist;
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("speed");
    p->SetValue = [](string val) {
        double prev = V_est;
        V_est = stod(val)/3.6;
        V_ura = 0.007*V_est;
        if (V_est < 0.2)
        {
            V_est = V_ura = 0;
        }
        if (prev != 0 && V_est == 0 && (mode == Mode::FS || mode == Mode::LS || mode == Mode::OS || mode == Mode::SR || mode == Mode::RV))
            position_report_reasons[0] = true;
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("time_offset");
    p->SetValue = [](string val) {
        TimeOffset::offset = atoi(val.c_str());
        set_persistent_command("timeOffset", val);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("acceleration");
    p->SetValue = [](string val) {
        A_est = stod(val);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("master_key");
    p->SetValue = [](string val) {
        cab_active[0] = cab_active[1] = false;
        if (val == "1") {
            if (current_odometer_orientation == 1)
                cab_active[0] = true;
            else
                cab_active[1] = true;
        }
    };
    manager.AddParameter(p);

    /*p = new ORserver::Parameter("other_master_key");
    p->SetValue = [](string val) {
        sl_signal = val == "1" && !cab_active[0] && !cab_active[1];
    };
    manager.AddParameter(p);*/

    p = new ORserver::Parameter("controller::direction");
    p->SetValue = [](string val) {
        reverser_direction = stoi(val);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("train_orientation");
    p->SetValue = [](string val) {
        current_odometer_orientation = stoi(val);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::telegram");
    p->SetValue = [](string val) {
        std::vector<unsigned char> message((val.size()+7)>>3);
        for (int i=0; i<val.size(); i++) {
            if (val[i]=='1')
                message[i>>3] |= 1<<(7-(i&7));
        }
        bit_manipulator r(std::move(message));
        eurobalise_telegram t(r);
        pending_telegrams.push_back({t,{distance(odometer_value-odometer_reference, odometer_orientation, 0), get_milliseconds()}});
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::level");
    p->GetValue = []() {
        return std::to_string((int)level);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::mode");
    p->GetValue = []() {
        return std::to_string((int)mode);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::emergency");
    p->GetValue = []() {
        return EB_command ? "true" : "false";
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::fullbrake");
    p->GetValue = []() {
        return SB_command ? "true" : "false";
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::tractioncutoff");
    p->GetValue = []() {
        return traction_cutoff_status ? "false" : "true";
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::neutral_section");
    p->GetValue = []() {
        return (neutral_section_info.start ? std::to_string(*neutral_section_info.start) : "")+";"+(neutral_section_info.end ? std::to_string(*neutral_section_info.end) : "");
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::lower_pantographs");
    p->GetValue = []() {
        return (lower_pantograph_info.start ? std::to_string(*lower_pantograph_info.start) : "")+";"+(lower_pantograph_info.end ? std::to_string(*lower_pantograph_info.end) : "");
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::atf");
    p->GetValue = []() {
        if (mode != Mode::FS) return std::string("-1");
        extern MonitoringStatus monitoring;
        std::shared_ptr<target> t = (monitoring == CSM) ? indication_target : MRDT;
        if (t != nullptr) {
            //t->calculate_curves();
            double speed = t->get_target_speed();
            double dist;
            if (t->type == target_class::EoA || t->type == target_class::SvL)
                dist = std::max(std::min(*EoA-d_estfront, *SvL-d_maxsafefront(*SvL)), 0.0);
            else
                dist = std::max(t->get_target_position()-d_maxsafefront(t->get_target_position()), 0.0);//std::max(t->d_P-d_maxsafefront(t->get_target_position()), 0.0);*/
            float atf = std::max(speed, std::min(sqrt(2*0.4f*(dist-V_est*10-10) + speed * speed)-3/3.6, V_perm));
            return std::to_string(atf);
        }
        else return std::to_string(V_perm);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("cruise_speed");
    p->SetValue = [](string val) {
        V_set = stod(val)/3.6;
    };
    manager.AddParameter(p);
    
    p = new ORserver::Parameter("etcs::vperm");
    p->GetValue = []() {
        return std::to_string(V_perm*3.6);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::vtarget");
    p->GetValue = []() {
        return std::to_string(V_target*3.6);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::vsbi");
    p->GetValue = []() {
        return std::to_string(V_sbi*3.6);
    };
    manager.AddParameter(p);
    
    p = new ORserver::Parameter("etcs::supervision");
    p->GetValue = []() {
        string s = "";
        extern SupervisionStatus supervision;
        switch(supervision)
        {
            case NoS:
                s += "NoS";
                break;
            case IndS:
                s += "IndS";
                break;
            case OvS:
                s += "OvS";
                break;
            case WaS:
                s += "WaS";
                break;
            case IntS:
                s += "IntS";
                break;
        }
        return s;
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::dmi::feedback");
    p->SetValue = [](string val) {
        parse_command(val);
    };

    p = new ORserver::Parameter("stm::command");
    p->SetValue = [](std::string val) {
        bit_manipulator r(val);
        stm_message msg(r);
        handle_stm_message(msg);
        /*for (auto &var : r.log_entries)
        {
            std::cout<<var.first<<"\t"<<var.second<<"\n";
        }*/
        send_command("stmData", val);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("stm::lzb::isolated");
    p->SetValue = [](std::string val) {
        auto it = installed_stms.find(10);
        if (it != installed_stms.end())
            it->second->isolated = val == "1";
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::isolated");
    p->SetValue = [](std::string val) {
        isolated = val == "1";
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::failed");
    p->SetValue = [](std::string val) {
        failed = val == "1";
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("serie");
    p->SetValue = [](std::string val) {
        load_config(val);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("language");
    p->SetValue = [](std::string val) {
        set_language(val);
    };
    manager.AddParameter(p);

    // TODO: directly connect to DMI instead of forwarding button state
    p = new ORserver::Parameter("ackButton");
    p->SetValue = [](std::string val) {
        send_command("ackButton", val);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::button::ack");
    p->SetValue = [](std::string val) {
        send_command("ackButton", val);
    };
    manager.AddParameter(p);

    p = new ORserver::Parameter("etcs::button::ack::light");
    p->GetValue = []() {
        return std::to_string(ack_button_light);
    };
    manager.AddParameter(p);

    for (int i=1; i<11; i++) {
        p = new ORserver::Parameter("etcs::button::f"+std::to_string(i));
        p->SetValue = [i](std::string val) {
            send_command("keyF"+std::to_string(i), val);
        };
        manager.AddParameter(p);
    }

    for (int i=1; i<8; i++) {
        p = new ORserver::Parameter("etcs::button::h"+std::to_string(i));
        p->SetValue = [i](std::string val) {
            send_command("keyH"+std::to_string(i), val);
        };
        manager.AddParameter(p);
    }

    p = new ORserver::Parameter("gsmr::active");
    p->SetValue = [](std::string val) {
        for (auto *t : mobile_terminals) {
            t->powered = val != "0";
        }
    };
    manager.AddParameter(p);
}

std::unique_ptr<ORTSClientWrapper> sim_wrapper;
std::unique_ptr<BasePlatform::BusSocket> sim_socket;
std::vector<std::string> registered_params;
void sim_write_line(const std::string &str)
{
    if (sim_socket)
        sim_socket->broadcast(BasePlatform::BusSocket::PeerId::fourcc("SRV"), str);
}

void register_parameter(std::string param)
{
    registered_params.push_back(param);
}

void sim_receive(BasePlatform::BusSocket::Message &&msg)
{
    manager.ParseLine(sim_wrapper.get(), msg.data);
}

void sim_receive(BasePlatform::BusSocket::JoinNotification &&msg)
{
    if (msg.peer.tid == BasePlatform::BusSocket::PeerId::fourcc("SRV"))
        for (const auto &param : registered_params)
            sim_socket->send_to(msg.peer.uid, "register(" + param + ")");
}

void sim_receive(BasePlatform::BusSocket::LeaveNotification &&msg) {
}

void sim_receive_handler(BasePlatform::BusSocket::ReceiveResult &&result)
{
    sim_socket->receive().then(sim_receive_handler).detach();
    std::visit([](auto&& arg){ sim_receive(std::move(arg)); }, std::move(result));
}

void orts_start();

void start_or_iface()
{
    orts_start();

    sim_socket = platform->open_socket("evc_sim", BasePlatform::BusSocket::PeerId::fourcc("EVC"));
    if (!sim_socket)
        return;

    sim_socket->receive().then(sim_receive_handler).detach();
    sim_wrapper = std::make_unique<ORTSClientWrapper>(*sim_socket, BasePlatform::BusSocket::PeerId::fourcc("SRV"), false);

    SetParameters();

    register_parameter("time_offset");
    register_parameter("ackButton");
    register_parameter("etcs::button::*");
    register_parameter("speed");
    register_parameter("distance");
    register_parameter("acceleration");
    register_parameter("etcs::telegram");
    register_parameter("cruise_speed");
    register_parameter("etcs::dmi::feedback");
    register_parameter("master_key");
    register_parameter("controller::direction");
    register_parameter("train_orientation");
    register_parameter("stm::command");
    register_parameter("stm::+::isolated");
    register_parameter("etcs::isolated");
    register_parameter("etcs::failed");
    register_parameter("gsmr::active");
    register_parameter("serie");
    register_parameter("language");
}

void update_or_iface()
{
    std::for_each(manager.parameters.begin(), manager.parameters.end(), [](ORserver::Parameter* p){p->Send();});
}
