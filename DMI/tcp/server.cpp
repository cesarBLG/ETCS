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
#include <SDL2/SDL.h>
#include <string>
#include <cstdio>
#include <thread>
#include <chrono>
#include <set>
#include <cmath>
#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif
#include "server.h"
#include "../monitor.h"
#include "../sound/sound.h"
#include "../messages/messages.h"
#include "../time.h"
#include "../planning/planning.h"
#include "../graphics/drawing.h"
#include "../state/gps_pos.h"
#include <mutex>
using namespace std;
int server;
int client;
#define BUFF_SIZE 1024
#define PORT 5010
static char data[BUFF_SIZE];
string buffer;
static SDL_Event ev;
void notifyDataReceived()
{
    //repaint();
}
#include <iostream>
void parseData(string str)
{
    extern mutex draw_mtx;
    unique_lock<mutex> lck(draw_mtx);
    int index = str.find_first_of('(');
    string command = str.substr(0, index);
    string value = str.substr(index+1, str.find_first_of(')')-index-1);
    if(command == "setVperm") Vperm = stof(value);
    if(command == "setVsbi") Vsbi = stof(value);
    if(command == "setVtarget") Vtarget = stof(value);
    if(command == "setVest") Vest = stof(value);
    if(command == "setVrelease") Vrelease = stof(value);
    if(command == "setVset") Vset = stof(value);
    if(command == "setDtarget") Dtarg = stof(value);
    if(command == "setTTI") TTI = stof(value);
    if(command == "setTTP") TTP = stof(value);
    if(command == "setLevel") level = value== "NTC" ? Level::NTC : (Level)stoi(value);
    if(command == "setLevelTransition") {
        if (value.empty()) {
            levelAck = 0;
        } else {
            int ind = value.find_first_of(',');
            string lev = value.substr(0,ind);
            ackLevel = lev== "NTC" ? Level::NTC : (Level)stoi(lev);
            levelAck = stoi(value.substr(ind+1));
        }
    }
    if(command == "setEB") EB = value!="false";
    if(command == "setOverride") ovEOA = value!="false";
    if(command == "setMode")
    {
        Mode m=mode;
        if(value == "FS") m = Mode::FS;
        else if(value == "LS") m = Mode::LS;
        else if(value == "OS") m = Mode::OS;
        else if(value == "SH") m = Mode::SH;
        else if(value == "SB") m = Mode::SB;
        else if(value == "SR") m = Mode::SR;
        else if(value == "UN") m = Mode::UN;
        else if(value == "TR") m = Mode::TR;
        else if(value == "PT") m = Mode::PT;
        mode = m;
    }
    if(command == "setModeTransition")
    {
        Mode m;
        modeAck = true;
        if (value.empty()) modeAck = false;
        else if(value == "FS") m = Mode::FS;
        else if(value == "LS") m = Mode::LS;
        else if(value == "OS") m = Mode::OS;
        else if(value == "SH") m = Mode::SH;
        else if(value == "SB") m = Mode::SB;
        else if(value == "SR") m = Mode::SR;
        else if(value == "UN") m = Mode::UN;
        else if(value == "TR") m = Mode::TR;
        ackMode = m;
    }
    if(command == "setMonitor") setMonitor(value == "CSM" ? CSM : (value == "TSM" ? TSM : RSM));
    if(command == "setSupervision")
    {
        SupervisionStatus s=IntS;
        if(value == "NoS") s = NoS;
        else if(value == "IndS") s = IndS;
        else if(value == "OvS") s = OvS;
        else if(value == "WaS") s = WaS;
        else if(value == "IntS") s = IntS;
        setSupervision(s);
    }
    if(command == "setGeoPosition") pk = stof(value);
    if(command == "setPlanningObjects")
    {
        planning_elements.clear();
        if (value.empty())
            return;

        vector<string> val;

        int valsep = value.find(',');
        while(valsep!=string::npos)
        {
            string param = value.substr(0,valsep);
            value = value.substr(valsep+1);
            valsep = value.find(',');
            val.push_back(param);
        }
        if (!value.empty()) val.push_back(value);

        for(int i = 0; i < val.size(); i+=2)
        {
            planning_element p;
            p.condition = stoi(val[i]);
            p.distance = stoi(val[i+1]);
            planning_elements.push_back(p);
        }
    }
    if(command == "setPlanningGradients")
    {
        vector<string> val;

        int valsep = value.find(',');
        while(valsep!=string::npos)
        {
            string param = value.substr(0,valsep);
            value = value.substr(valsep+1);
            valsep = value.find(',');
            val.push_back(param);
        }
        if (!value.empty()) val.push_back(value);

        gradient_elements.clear();
        for(int i = 0; i < val.size(); i+=2)
        {
            gradient_element g;
            g.distance = (int)round(stof(val[i]));
            g.val = (int)round(stof(val[i+1]));;
            gradient_elements.push_back(g);
        }
    }
    if(command == "setPlanningSpeeds")
    {
        extern bool planning_unchanged;
        vector<string> val;

        int valsep = value.find(',');
        while(valsep!=string::npos)
        {
            string param = value.substr(0,valsep);
            value = value.substr(valsep+1);
            valsep = value.find(',');
            val.push_back(param);
        }
        if (!value.empty()) val.push_back(value);

        speed_elements.clear();
        bool imark=false;
        for(int i = 0; i < val.size(); i+=2)
        {
            speed_element s;
            bool isim=false;
            int inddist;
            if(val[i]=="im")
            {
                i++;
                isim = true;
                imark = true;
                inddist = (int)round(stof(val[i]));
                i++;
            }
            s.distance = (int)round(stof(val[i]));
            s.speed = (int)round(stof(val[i+1]));
            if(isim)
            {
                imarker.element = s;
                imarker.start_distance=inddist;
            }
            speed_elements.push_back(s);
        }
        if(!imark) imarker.start_distance = 0;
        planning_unchanged = false;
    }
    if(command == "setActiveConditions")
    {
        vector<string> val;

        int valsep = value.find(',');
        while(valsep!=string::npos)
        {
            string param = value.substr(0,valsep);
            value = value.substr(valsep+1);
            valsep = value.find(',');
            val.push_back(param);
        }
        if (!value.empty()) val.push_back(value);

        std::set<int> syms;
        for(int i = 0; i<val.size(); i++)
        {
            syms.insert(stoi(val[i]));
        }
        extern int track_conditions[];
        std::set<int> asig;
        for (int i=0; i<3; i++) 
        {
            if (syms.find(track_conditions[i]) == syms.end()) track_conditions[i] = 0;
            if (track_conditions[i] != 0) asig.insert(track_conditions[i]);
        }
        for (int s : syms)
        {
            if (asig.size()>=3) break;
            if (asig.find(s) != asig.end()) continue;
            for (int i=0; i<3; i++)
            {
                if (track_conditions[i] == 0)
                {
                    track_conditions[i] = s;
                    asig.insert(s);
                    break;
                }
            }
        }
    }
    if (command == "setMessage")
    {
        int valsep = value.find(',');
        int id = stoi(value.substr(0,valsep));
        value = value.substr(valsep+1);
        valsep = value.find(',');
        int size = stoi(value.substr(0,valsep));
        value = value.substr(valsep+1);
        string text = value.substr(0, size);

        value = value.substr(size+1);
        valsep = value.find(',');
        vector<string> val;
        while(valsep!=string::npos)
        {
            string param = value.substr(0,valsep);
            value = value.substr(valsep+1);
            valsep = value.find(',');
            val.push_back(param);
        }
        val.push_back(value);
        addMsg(Message(id, text, stoi(val[0]), stoi(val[1]), val[2]!="false", val[3]!="false", stoi(val[4])));
    }
    if (command == "setRevokeMessage")
    {
        revokeMessage(stoi(value));
    }
    if (command == "playSinfo") playSinfo();
    update();
    notifyDataReceived();
}
extern bool running;
void read()
{
    int result = recv(client, data, BUFF_SIZE-1, 0);
    if(result>0)
    {
        data[result] = 0;
        buffer += data;
        int end;
        while ((end=buffer.find_first_of(';'))!=string::npos) {
            int start = buffer.find_first_not_of("\n\r ;");
            string command = buffer.substr(start, end-start);
            parseData(command);
            buffer = buffer.substr(end+1);
        }
    }
    if (result < 0) running = false;
}
void write_command(string command, string value)
{
    string tosend = command+"("+value+");\n";
    send(client, tosend.c_str(), tosend.size(), 0);
}
void startSocket()
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif
    server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    serv.sin_addr.s_addr = INADDR_ANY;
    if (server == -1) {
        perror("socket");
        return;
    }
    if (0 != ::bind(server, (struct sockaddr *)&(serv), sizeof(serv))) {
        perror("bind");
        return;
    }
    if (listen(server, 1) == -1) {
        perror("listen");
        return;
    }
    client = -1;
    while(client == -1 && running)
    {
        struct sockaddr_in addr;
        int c = sizeof(struct sockaddr_in);
        int cl = accept(server, (struct sockaddr *)&addr, 
#ifdef _WIN32
        (int *)
#else
        (socklen_t *)
#endif
        &c);
        if(cl == -1) {
            perror("accept");
            continue;
        }
        client = cl;
    }
    while(running)
    {
        read();
    }
    closeSocket();
}
void closeSocket()
{
#ifdef _WIN32
    closesocket(server);
#else
    close(server);
#endif
}
