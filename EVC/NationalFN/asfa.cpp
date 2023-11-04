/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "asfa.h"
#include "../Supervision/supervision.h"
#include <orts/common.h>
#include "../DMI/text_message.h"
#include "../Time/clock.h"
#include "../Procedures/level_transition.h"
#include "../STM/stm.h"
#include <string>
using namespace ORserver;
using std::string;
extern ParameterManager manager;
bool AKT=false;
bool CON=true;
bool CON_LZB=false;
bool detected = false;
bool connected = false;
bool active = false;
bool brake_commanded = false;
void register_parameter(std::string parameter);
void initialize_asfa()
{
    Parameter *p;
    p = new Parameter("asfa::akt::etcs");
    p->GetValue = []() {
        return AKT ? "1" : "0";
    };
    manager.AddParameter(p);

    p = new Parameter("asfa::con::etcs");
    p->GetValue = []() {
        return CON ? "1" : "0";
    };
    manager.AddParameter(p);

    p = new Parameter("asfa::conectado");
    p->SetValue = [](string val) {
        detected = val.length() > 0;
        connected = val == "1";
    };
    manager.AddParameter(p);

    p = new Parameter("asfa::con::lzb");
    p->SetValue = [](string val) {
        CON_LZB = val == "1";
    };
    manager.AddParameter(p);

    register_parameter("asfa::conectado");
    register_parameter("asfa::con::lzb");
}
extern double V_NVUNFIT;
#include "../Position/distance.h"
#include "../Supervision/speed_profile.h"
extern optional<speed_restriction> UN_speed;
int64_t akt_delay=0;
int64_t con_delay=0;
text_message &send_msg()
{
    bool con = CON;
    std::string nivel="";
    if (level == Level::N0 || (ongoing_transition && ongoing_transition->leveldata.level == Level::N0)) nivel = "Nivel 0 - ";
    else if ((level == Level::NTC && nid_ntc == 0) || (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && ongoing_transition->leveldata.nid_ntc == 0)) nivel = "";
    else if (level == Level::N1) nivel = "Nivel 1 - ";
    else if (level == Level::N2) nivel = "Nivel 2 - ";
    else if (level == Level::N3) nivel = "Nivel 3 - ";
    int64_t time = get_milliseconds();
    return add_message(text_message(nivel + (connected ? "ASFA conectado en C.G." : "ASFA anulado en C.G."), false, false, false, [time,con](text_message &m) {
        return !brake_commanded && (time+30000<get_milliseconds() || con != CON);
    }));
}
void update_stm_asfa()
{
    if (installed_stms.find(10) == installed_stms.end()) return;
    if (level == Level::NTC && nid_ntc == 10 && installed_stms[10]->state == stm_state::DA && CON_LZB) {
        level_information lv = {Level::NTC, 0};
        stm_level_change(lv, true);
        nid_ntc = 0;
        if (!ongoing_transition) {
            ongoing_transition = level_transition_information();
            ongoing_transition->immediate = false;
            ongoing_transition->dist = 0;
            ongoing_transition->acknowledged = true;
            ongoing_transition->leveldata = {0.0, Level::NTC, 10};
            ongoing_transition->ref_loc = distance::from_odometer(dist_base::max);
            level_acknowledgeable = false;
            level_acknowledged = true;
            level_to_ack = Level::NTC;
            ntc_to_ack = 10;
            assign_stm(10, false);
        }
    }
    if (level != Level::NTC || nid_ntc != 0) return;
    if (ongoing_transition && ongoing_transition->leveldata.level == Level::NTC && ongoing_transition->leveldata.nid_ntc == 10 && installed_stms[10]->state == stm_state::HS && !CON_LZB) {
        ongoing_transition = {};
        level_information lv = {Level::NTC, 10};
        stm_level_change(lv, false);
        nid_ntc = 10;
    }
}
void update_asfa()
{
    bool stm = installed_stms.find(0) != installed_stms.end() || installed_stms.find(2) != installed_stms.end() || installed_stms.find(19) != installed_stms.end();
    if (stm) {
        AKT = false;
        CON = true;
        active = false;
        brake_commanded = false;
        update_stm_asfa();
        return;
    }
    if (!detected || mode == Mode::IS || mode == Mode::SL || mode == Mode::NP || level == Level::Unknown || !level_valid) {
        AKT = false;
        CON = true;
        active = false;
        brake_commanded = false;
        return;
    }
    bool msg = false;
    if (!active) {
        active = true;
        msg = true;
    }
    if (!connected && ((level == Level::N0 && ntc_available_no_stm.find(0) == ntc_available_no_stm.end()) || (level == Level::NTC && nid_ntc == 0))) {
        CON = false;
        AKT = true;
        if (!brake_commanded) {
            brake_commanded = true;
            auto &msg = send_msg();
            brake_conditions.push_back({10, &msg, [](brake_command_information &i) {
                return !brake_commanded;
            }});
        }
        return;
    } else {
        brake_commanded = false;
    }
    if (ongoing_transition && ((ongoing_transition->leveldata.level == Level::N0 && ntc_available_no_stm.find(0) == ntc_available_no_stm.end()) || (ongoing_transition->leveldata.level == Level::NTC && ongoing_transition->leveldata.nid_ntc == 0))) {
        if (!CON) {
            CON = true;
            akt_delay = get_milliseconds();
            msg = true;
        }
    } else if ((level == Level::N0 && ntc_available_no_stm.find(0) == ntc_available_no_stm.end()) || (level == Level::NTC && (nid_ntc == 0 || nid_ntc == 10))) {
        if (!CON) {
            CON = true;
            akt_delay = get_milliseconds();
            msg = true;
        } else if (AKT && get_milliseconds()-akt_delay > 500)
            AKT = false;
    } else {
        if (CON) {
            CON = false;
            msg = true;
        }
        AKT = true;
    }
    if (msg)
        send_msg();
}
