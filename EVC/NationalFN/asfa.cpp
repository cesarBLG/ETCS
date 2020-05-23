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
#include <string>
#include <mutex>
using namespace ORserver;
using namespace std;
extern ParameterManager manager;
bool AKT=false;
bool CON=true;
extern mutex loop_mtx;
extern mutex iface_mtx;
bool detected = false;
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

    p = new Parameter("asfa::cg");
    p->SetValue = [](string val) {
        detected = val=="1";
        if (detected)
            add_message(text_message("ASFA conectado en C.G.", false, false, false, [](text_message &t){return !detected;}));
    };
    manager.AddParameter(p);
}
void update_asfa()
{
    if (!detected)
        return;
    if (level == Level::N0 && mode == Mode::UN) {
        AKT = false;
        CON = true;
    } else if (level != Level::N0 && level != Level::Unknown && mode != Mode::UN) {
        AKT = true;
        CON = false;
    }
}