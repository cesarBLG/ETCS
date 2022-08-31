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
#include "../time_etcs.h"
#include "../planning/planning.h"
#include "../graphics/drawing.h"
#include "../state/gps_pos.h"
#include "../state/acks.h"
#include "../control/control.h"
#include <mutex>
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
template<class T>
void fill_non_existent(json &j, std::string str, T def)
{
    if (!j.contains("str" || j.is_null())) j[str] = def;
}
void fill_non_existent(json &j, std::string str, nullptr_t null)
{
    if (!j.contains("str")) j[str] = nullptr;
}
void from_json(const json&j, speed_element &e)
{
    e.distance = j["DistanceToTrainM"].get<double>();
    e.speed = std::round(j["TargetSpeedMpS"].get<double>()*3.6);
}
void from_json(const json&j, gradient_element &e)
{
    e.distance = j["DistanceToTrainM"].get<double>();
    e.val = j["GradientPerMille"].get<double>();
}
enum struct TrackConditionType
{
    Custom,
    LowerPantograph,
    RaisePantograph,
    NeutralSectionAnnouncement,
    EndOfNeutralSection,
    NonStoppingArea,
    RadioHole,
    MagneticShoeInhibition,
    EddyCurrentBrakeInhibition,
    RegenerativeBrakeInhibition,
    OpenAirIntake,
    CloseAirIntake,
    SoundHorn,
    TractionSystemChange,
    // Legacy conditions used in old DMIs
    Tunnel,
    Bridge,
    Station,
    EndOfTrack
};
enum struct TractionSystem
{
    NonFitted,
    AC25kV,
    AC15kV,
    DC3000V,
    DC1500V,
    DC750V
};
void from_json(const json&j, planning_element &e)
{
    e.distance = j["DistanceToTrainM"].get<double>();
    TrackConditionType type = (TrackConditionType)j["Type"].get<int>();
    bool yellow = j["YellowColour"].get<bool>();
    int tex = 0;
    switch(type)
    {
        case TrackConditionType::LowerPantograph:
            tex = yellow ? 2 : 1;
            break;
        case TrackConditionType::RaisePantograph:
            tex = yellow ? 4 : 3;
            break;
        case TrackConditionType::NeutralSectionAnnouncement:
            tex = yellow ? 6 : 5;
            break;
        case TrackConditionType::EndOfNeutralSection:
            tex = yellow ? 8 : 7;
            break;
        case TrackConditionType::NonStoppingArea:
            tex = 9;
            break;
        case TrackConditionType::RadioHole:
            tex = 10;
            break;
        case TrackConditionType::MagneticShoeInhibition:
            tex = yellow ? 12 : 11;
            break;
        case TrackConditionType::EddyCurrentBrakeInhibition:
            tex = yellow ? 14 : 13;
            break;
        case TrackConditionType::RegenerativeBrakeInhibition:
            tex = yellow ? 16 : 15;
            break;
        case TrackConditionType::CloseAirIntake:
            tex = yellow ? 19 : 17;
            break;
        case TrackConditionType::OpenAirIntake:
            tex = yellow ? 20 : 18;
            break;
        case TrackConditionType::SoundHorn:
            tex = 35;
            break;
        case TrackConditionType::TractionSystemChange:{
            TractionSystem traction = (TractionSystem)j["TractionSystem"].get<int>();
            switch(traction)
            {
                case TractionSystem::NonFitted:
                    tex = yellow ? 26 : 25;
                    break;
                case TractionSystem::AC25kV:
                    tex = yellow ? 28 : 27;
                    break;
                case TractionSystem::AC15kV:
                    tex = yellow ? 30 : 29;
                    break;
                case TractionSystem::DC3000V:
                    tex = yellow ? 32 : 31;
                    break;
                case TractionSystem::DC1500V:
                    tex = yellow ? 34 : 33;
                    break;
                case TractionSystem::DC750V:
                    tex = yellow ? 36 : 35;
                    break;
                default:
                    break;
            }
            break;}
        case TrackConditionType::Tunnel:
            tex = 40;
            break;
        case TrackConditionType::Bridge:
            tex = 41;
            break;
        case TrackConditionType::Station:
            tex = 42;
            break;
        case TrackConditionType::EndOfTrack:
            tex = 43;
            break;
        default:
            break;
    }
    e.condition = tex;
}
void parseData(string str)
{
    extern mutex draw_mtx;
    unique_lock<mutex> lck(draw_mtx);
    int index = str.find_first_of('(');
    string command = str.substr(0, index);
    string value = str.substr(index+1, str.find_first_of(')')-index-1);
    if (command == "setMessage")
    {
        int valsep = value.find(',');
        unsigned int id = stoi(value.substr(0,valsep));
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
    if (command != "json") return;
    json j = json::parse(value);
    /*fill_non_existent(j, "TargetSpeedMpS", 1000);
    fill_non_existent(j, "ReleaseSpeedMpS", 0);
    fill_non_existent(j, "ModeAcknowledgement", nullptr);
    fill_non_existent(j, "LevelTransition", nullptr);*/
    Vperm = j["AllowedSpeedMpS"].get<double>()*3.6;
    Vtarget = j["TargetSpeedMpS"].get<double>()*3.6;
    Vsbi = j["InterventionSpeedMpS"].get<double>()*3.6;
    Dtarg = j["TargetDistanceM"].get<double>();
    Vrelease = round(j["ReleaseSpeedMpS"].get<double>()*3.6);
    Vest = j["SpeedMpS"].get<double>()*3.6;
    TTP = j["TimeToPermittedS"].get<double>();
    TTI = j["TimeToIndicationS"].get<double>();
    setMonitor((MonitoringStatus)j["CurrentMonitoringStatus"].get<int>());
    setSupervision((SupervisionStatus)j["CurrentSupervisionStatus"].get<int>());
    mode = (Mode)j["CurrentMode"].get<int>();
    level = (Level)j["CurrentLevel"].get<int>();
    if (level == Level::NTC) nid_ntc = j["CurrentNTC"].get<int>();
    if (j["GeographicalPositionKM"].is_null()) pk = -1;
    else pk = j["GeographicalPositionKM"].get<double>();
    if (!j["ModeAcknowledgement"].is_null())
    {
        ackMode = (Mode)j["ModeAcknowledgement"].get<int>();
        setAck(AckType::Mode, 0, true);
    }
    else
    {
        setAck(AckType::Mode, 0, false);
    }
    if (!j["LevelTransition"].is_null())
    {
        ackLevel = (Level)j["LevelTransition"]["Level"].get<int>();
        if (ackLevel == Level::NTC) ackNTC = j["LevelTransition"]["NTC"].get<int>();
        setAck(AckType::Level, j["LevelTransition"]["Acknowledge"].get<bool>()+1, true);
    }
    else
    {
        setAck(AckType::Level, 0, false);
    }
    ovEOA = j["OverrideActive"].get<bool>();
    radioStatus = j["RadioStatus"].get<int>();
    EB = SB = j["BrakeCommanded"].get<bool>();
    extern bool display_taf;
    display_taf = j["DisplayTAF"].get<bool>();
    setWindow(j["ActiveWindow"]);
    setAck(AckType::Brake, 0, j["BrakeAcknowledge"].get<bool>());
    {
        extern bool planning_unchanged;
        speed_elements = j["SpeedTargets"].get<std::vector<speed_element>>();
        if (j["IndicationMarkerDistanceM"].is_null()) imarker.start_distance = 0;
        else imarker.start_distance = j["IndicationMarkerDistanceM"].get<double>();
        if (!j["IndicationMarkerTarget"].is_null()) imarker.element = j["IndicationMarkerTarget"].get<speed_element>();
        planning_unchanged = false;
        gradient_elements = j["GradientProfile"].get<std::vector<gradient_element>>();
        planning_elements = j["PlanningTrackConditions"].get<std::vector<planning_element>>();
        
    }
    {
        std::set<int> syms = j["ActiveTrackConditions"].get<std::set<int>>();
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
    /*if(command == "setVset") Vset = stof(value);*/
    if (command == "playSinfo") playSinfo();
    update();
    notifyDataReceived();
}
extern bool running;
void read()
{
    int result = recv(client, ::data, BUFF_SIZE-1, 0);
    if(result>0)
    {
        ::data[result] = 0;
        buffer += ::data;
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
