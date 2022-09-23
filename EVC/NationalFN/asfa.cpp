/*
 * European Train Control System
 * Copyright (C) 2020  César Benito <cesarbema2009@hotmail.com>
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
#include "asfa.h"
#include "../Supervision/supervision.h"
#include <orts/common.h>
#include "../DMI/text_message.h"
#include "../Time/clock.h"
#include "../Procedures/level_transition.h"
#include <string>
#include <mutex>
using namespace ORserver;
using std::mutex;
using std::string;
extern ParameterManager manager;
bool AKT=false;
bool CON=true;
extern mutex loop_mtx;
extern mutex iface_mtx;
bool detected = false;
bool connected = false;
bool active = false;
bool brake_commanded = false;
void register_parameter(std::string parameter);
void initialize_asfa()
{
    std::unique_lock<mutex> lck(iface_mtx);
    Parameter *p;
    /*p = new Parameter("asfa::akt");
    p->GetValue = []() {
        return AKT ? "1" : "0";
    };
    manager.AddParameter(p);

    p = new Parameter("asfa::con");
    p->GetValue = []() {
        return CON ? "1" : "0";
    };
    manager.AddParameter(p);*/

    p = new Parameter("asfa::conectado");
    p->SetValue = [](string val) {
        detected = val.length() > 0;
        connected = val == "1";
    };
    manager.AddParameter(p);

    register_parameter("asfa::conectado");
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
void update_asfa()
{
    if (!detected || mode == Mode::IS || mode == Mode::SL || mode == Mode::NP || level == Level::Unknown) {
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
    if (!connected && level == Level::NTC && nid_ntc == 0) {
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
    if (ongoing_transition && (ongoing_transition->leveldata.level == Level::N0 || (ongoing_transition->leveldata.level == Level::NTC && ongoing_transition->leveldata.nid_ntc == 0))) {
        if (!CON) {
            CON = true;
            akt_delay = get_milliseconds();
            msg = true;
        }
    } else if (level == Level::N0 || (level == Level::NTC && nid_ntc == 0)) {
        if (!CON) {
            CON = true;
            akt_delay = get_milliseconds();
            msg = true;
        }
        else if (AKT && get_milliseconds()-akt_delay > 500)
            AKT = false;
    } else {
        if (CON) {
            if (!AKT)
                con_delay = get_milliseconds();
            if (get_milliseconds()-con_delay > 500) {
                CON = false;
                msg = true;
            }
        }
        AKT = true;
    }
    if (msg)
        send_msg();
}
