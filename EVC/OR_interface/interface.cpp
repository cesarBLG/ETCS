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
#include "interface.h"
#include <orts/client.h>
#include <orts/common.h>
#include <set>
#include <bitset>
#include <algorithm>
#include "../Position/distance.h"
#include "../Supervision/speed_profile.h"
#include "../MA/movement_authority.h"
#include "../Position/linking.h"
#include "../Packets/messages.h"
#include "../TrainSubsystems/power.h"
#include <iostream>
#include <thread>
#include <mutex>
using namespace ORserver;
using std::string;
using std::cout;
using std::endl;
using std::thread;
using std::mutex;
extern mutex loop_mtx;
extern double V_est;
double V_set;
extern distance d_estfront;
extern bool EB_commanded;
extern bool SB_commanded;
double or_dist;
POSIXclient *s_client;
ParameterManager manager;
mutex iface_mtx;
void SetParameters()
{
    std::unique_lock<mutex> lck(iface_mtx);
    Parameter *p = new Parameter("distance");
    p->SetValue = [](string val) {
        or_dist = stof(val);
        odometer_value = or_dist;
    };
    manager.AddParameter(p);

    p = new Parameter("speed");
    p->SetValue = [](string val) {
        V_est = stof(val)/3.6;
        V_ura = 0.007*V_est;
        if (V_est < 0.2)
        {
            V_est = V_ura = 0;
        }
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::telegram");
    p->SetValue = [](string val) {
        std::vector<bool> message;
        for (int i=0; i<val.size(); i++) {
            message.push_back(val[i]=='1');
        }
        bit_read_temp r(message);
        eurobalise_telegram t(r);
        pending_telegrams.push_back(t);
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::emergency");
    p->GetValue = []() {
        return EB_commanded ? "true" : "false";
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::fullbrake");
    p->GetValue = []() {
        return SB_commanded ? "true" : "false";
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::tractioncutoff");
    p->GetValue = []() {
        return traction_cutoff_status ? "false" : "true";
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::main_power_switch");
    p->GetValue = []() {
        return main_power_switch_status ? "1" : "0";
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::pantographs");
    p->GetValue = []() {
        return pantograph_status ? "1" : "0";
    };
    manager.AddParameter(p);

    p = new Parameter("cruise_speed");
    p->SetValue = [](string val) {
        V_set = stof(val)/3.6;
    };
    manager.AddParameter(p);
}
void polling()
{
    threadwait *poller = new threadwait();
    s_client = new TCPclient("127.0.0.1", 5090, poller);//TCPclient::connect_to_server(poller);
    s_client->WriteLine("register(speed)");
    s_client->WriteLine("register(distance)");
    s_client->WriteLine("register(etcs::telegram)");
    s_client->WriteLine("register(cruise_speed)");
    s_client->WriteLine("register(asfa::cg)");
    SetParameters();
    while(s_client->connected) {
        int nfds = poller->poll(100);
        s_client->handle();
        string s = s_client->ReadLine();
        std::unique_lock<mutex> lck(iface_mtx);
        std::unique_lock<mutex> lck2(loop_mtx);
        while(s!="") {
            manager.ParseLine(s_client, s);
            s = s_client->ReadLine();
        }
        std::for_each(manager.parameters.begin(), manager.parameters.end(), [](Parameter* p){p->Send();});
    }
}
void start_or_iface()
{
    thread t(polling);
    t.detach();
}
