/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
#include "../STM/stm_objects.h"
#include "../language/language.h"
#include "../speed/gauge.h"
#include "../Config/config.h"
#include <mutex>
int server;
int clients[3];
int active_channel;
#define BUFF_SIZE 1024
#define PORT 5010
static char data[BUFF_SIZE];
std::string buffer;
std::mutex server_mtx;
static SDL_Event ev;
#include <iostream>
template<class T>
void fill_non_existent(json &j, std::string str, T def)
{
    if (!j.contains("str") || j.is_null()) j[str] = def;
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
void parseData(std::string str)
{
    int index = str.find_first_of('(');
    std::string command = str.substr(0, index);
    std::string value = str.substr(index+1, str.find_last_of(')')-index-1);
    if (command == "setMessage")
    {
        int valsep = value.find(',');
        unsigned int id = stoi(value.substr(0,valsep));
        value = value.substr(valsep+1);
        valsep = value.find(',');
        int size = stoi(value.substr(0,valsep));
        value = value.substr(valsep+1);
        std::string text = value.substr(0, size);

        value = value.substr(size+1);
        valsep = value.find(',');
        std::vector<std::string> val;
        while(valsep!=std::string::npos)
        {
            std::string param = value.substr(0,valsep);
            value = value.substr(valsep+1);
            valsep = value.find(',');
            val.push_back(param);
        }
        val.push_back(value);
        addMsg(Message(id, text, stoi(val[0]), stoi(val[1]), val[2]!="false", val[3]!="false", stoi(val[4])));
    }
    else if (command == "setRevokeMessage")
    {
        revokeMessage(stoi(value));
    }
    else if (command == "playSinfo") playSinfo();
    else if (command == "stmData")
    {
        bit_manipulator r(value);
        stm_message msg(r);
        parse_stm_message(msg);
    }
    else if (command == "language")
    {
        set_language(value);
    }
    else if (command == "setSerie")
    {
        load_config(value);
    }
    else if (command == "setVset")
    {
        Vset = stof(value);
    }
    else if (command == "timeOffset")
    {
        TimeOffset::offset = atoi(value.c_str());
    }
    else if (command == "ackButton") {
        Component::externalAckButton(atoi(value.c_str()));
    }
    if (command != "json") return;
    json j = json::parse(value);
    setWindow(j);
    if (!j.contains("Status")) return;
    j = j["Status"];
    /*fill_non_existent(j, "TargetSpeedMpS", 1000);
    fill_non_existent(j, "ReleaseSpeedMpS", 0);
    fill_non_existent(j, "ModeAcknowledgement", nullptr);
    fill_non_existent(j, "LevelTransition", nullptr);*/
    if (level != Level::NTC || (mode != Mode::SN && mode != Mode::NL))
    {
        Vperm = (int)(j["AllowedSpeedMpS"].get<double>()*3.6+0.01);
        Vtarget = round(j["TargetSpeedMpS"].get<double>()*3.6);
        Vsbi = (int)(j["InterventionSpeedMpS"].get<double>()*3.6+0.01);
        Dtarg = round(j["TargetDistanceM"].get<double>());
        Vrelease = round(j["ReleaseSpeedMpS"].get<double>()*3.6);
    }
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
    if (j.contains("LSSMA")) setLSSMA((int)(j["LSSMA"].get<double>()*3.6 + 0.01));
    else setLSSMA(-1);
    setAck(AckType::Brake, 0, j["BrakeAcknowledge"].get<bool>());
    {
        speed_elements = j["SpeedTargets"].get<std::vector<speed_element>>();
        if (j["IndicationMarkerDistanceM"].is_null()) imarker.start_distance = 0;
        else imarker.start_distance = j["IndicationMarkerDistanceM"].get<double>();
        if (!j["IndicationMarkerTarget"].is_null()) imarker.element = j["IndicationMarkerTarget"].get<speed_element>();
        gradient_elements = j["GradientProfile"].get<std::vector<gradient_element>>();
        planning_elements = j["PlanningTrackConditions"].get<std::vector<planning_element>>();
        
    }
    {
        void updateTc(std::set<int> &syms);
        std::set<int> syms = j["ActiveTrackConditions"].get<std::set<int>>();
        updateTc(syms);
    }
}
extern bool running;
int read(int channel)
{
    int result = recv(clients[channel], ::data, BUFF_SIZE-1, 0);
    if(result>0)
    {
        ::data[result] = 0;
        std::unique_lock<std::mutex> lck(server_mtx);
        buffer += ::data;
    }
    return result;
}
void updateDrawCommands()
{
    std::unique_lock<std::mutex> lck(server_mtx);
    int end;
    while ((end=buffer.find_first_of(';'))!=std::string::npos) {
        int start = buffer.find_first_not_of("\n\r ;");
        std::string command = buffer.substr(start, end-start);
        parseData(command);
        buffer = buffer.substr(end+1);
    }
}
void write_command(std::string command, std::string value)
{
    if (active_channel < 0)
        return;
    std::string tosend = command+"("+value+");\n";
    send(clients[active_channel], tosend.c_str(), tosend.size(), 0);
}
void listenChannels()
{
    while(active_channel == -1 && running)
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
        for (int i=0; i<3; i++) {
            if (clients[i] < 0) {
                clients[i] = cl;
                active_channel = i;
                break;
            }
        }
    }
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
    if (listen(server, 3) == -1) {
        perror("listen");
        return;
    }
    active_channel = -1;
    for (int i=0; i<3; i++)
        clients[i] = -1;
}
void loopSocket()
{
    listenChannels();
    while(running)
    {
        int res = read(active_channel);
        if (res < 0)
        {
#ifdef _WIN32
            closesocket(clients[active_channel]);
#else
            close(clients[active_channel]);
#endif
            clients[active_channel] = -1;
            active_channel = -1;
            if (running) listenChannels();
        }
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
