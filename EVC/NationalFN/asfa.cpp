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
#include "../orts/common.h"
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
void register_parameter(std::string parameter);
void initialize_asfa()
{
    std::unique_lock<mutex> lck(iface_mtx);
    Parameter *p = new Parameter("asfa::akt");
    p->GetValue = []() {
        return AKT ? "1" : "0";
    };
    manager.AddParameter(p);

    p = new Parameter("asfa::con");
    p->GetValue = []() {
        return CON ? "1" : "0";
    };
    manager.AddParameter(p);

    p = new Parameter("asfa::conectado");
    p->SetValue = [](string val) {
        detected = val.length() > 0;
        connected = val == "1";
        if (detected) {
            int64_t time = get_milliseconds();
            add_message(text_message(connected ? "ASFA conectado en C.G." : "ASFA anulado en C.G.", false, false, false, [time](text_message &m) {
                return time+30000<get_milliseconds();
            }));
        }
    };
    manager.AddParameter(p);

    register_parameter("asfa::conectado");
}
extern double V_NVUNFIT;
#include "../Position/distance.h"
#include "../Supervision/speed_profile.h"
extern optional<speed_restriction> UN_speed;
int64_t akt_time=0;
void conectar_asfa()
{
    int64_t time = get_milliseconds();
    add_message(text_message(connected ? "ASFA conectado en C.G." : "ASFA anulado en C.G.", false, false, false, [time](text_message &m) {
        return time+30000<get_milliseconds();
    }));
    CON = true;
    akt_time = get_milliseconds();
}
void update_asfa()
{
    /*if (mode == Mode::UN) {
        double supervisionSpeed = detected && connected ? 200/3.6 : V_NVUNFIT;
        if (UN_speed->get_speed() != supervisionSpeed) {
            UN_speed = speed_restriction(supervisionSpeed, distance(std::numeric_limits<double>::lowest()), distance(std::numeric_limits<double>::max()), false);
            recalculate_MRSP();
        }
    }*/
    if (!detected) {
        return;
    }
    if (ongoing_transition && ongoing_transition->leveldata.level == Level::N0 && !CON) {
        conectar_asfa();
    }
    if (level == Level::N0) {
        if (!CON)
            conectar_asfa();
        if (akt_time == 0 || get_milliseconds()-akt_time > 500) AKT = false;
    } else {
        AKT = true;
        CON = false;
        akt_time = 0;
    }
}
