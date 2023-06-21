/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cstdlib>
#include <cstdio>
#include <string>
#include <sstream>
#include <list>
#include <algorithm>
#include <map>
#include "../Supervision/supervision.h"
#include "../Supervision/targets.h"
#include <iostream>
#include <chrono>
#include "../Supervision/train_data.h"
#include "../Supervision/speed_profile.h"
#include "../Euroradio/session.h"
#include "../Position/distance.h"
#include "../Position/geographical.h"
#include "../Procedures/override.h"
#include "../Procedures/mode_transition.h"
#include "../Procedures/level_transition.h"
#include "../TrackConditions/track_condition.h"
#include "track_ahead_free.h"
#include "text_message.h"
#include "windows.h"
#include "platform.h"

using std::map;
using std::set;
using std::list;
using std::string;
using std::to_string;
int dmi_pid;
std::unique_ptr<BasePlatform::BusSocket> dmi_socket;
void dmi_joined(BasePlatform::BusSocket::ClientId client);
void dmi_data_received(std::pair<BasePlatform::BusSocket::ClientId, std::string> &&data);
void dmi_update_func();
void start_dmi()
{
    dmi_socket = platform->open_socket("evc_dmi", BasePlatform::BusSocket::ClientId::fourcc("EVC_"));
    if (dmi_socket) {
        dmi_socket->on_peer_join().then(dmi_joined).detach();
        dmi_socket->receive().then(dmi_data_received).detach();
        platform->delay(100).then(dmi_update_func).detach();
    }
}
extern double V_target;
extern double V_perm;
extern double V_est;
extern double V_set;
extern double V_release;
extern double V_sbi;
extern double D_target;
extern double TTI;
extern double TTP;
extern bool EB_command;
extern bool SB_command;
extern MonitoringStatus monitoring;
extern SupervisionStatus supervision;

void parse_command(string str)
{
    int index = str.find_first_of('(');
    string command = str.substr(0, index);
    string value = str.substr(index+1, str.find_last_of(')')-index-1);
    if (command == "json")
    {
        json j = json::parse(value);
        if (j.contains("DriverSelection"))
        {
            std::string selection = j["DriverSelection"].get<std::string>();
            if (selection == "ModeAcknowledge") {
                mode_acknowledgeable = false;
                mode_acknowledged = true;
            } else if (selection == "LevelAcknowledge") {
                level_acknowledgeable = false;
                level_acknowledged = true;
            } else if (selection == "BrakeAcknowledge") {
                brake_acknowledgeable = false;
                brake_acknowledged = true;
            } else if (selection == "MessageAcknowledge") {
                message_acked(j["MessageId"]);
            } else if (selection == "FunctionRequest") {
                update_dialog_step(selection, "");
            } else if (selection == "ValidateDataEntry") {
                json &res = j["DataInputResult"];
                std::string window = j["WindowTitle"];
                validate_data_entry(window, res);
            } else if (selection == "ValidateEntryField") {
                std::string window = j["WindowTitle"];
                json &res = j["ValidateEntryField"];
                validate_entry_field(window, res);
            } else if (selection == "CloseWindow") {
                close_window();
            } else if (selection == "TrackAheadFree") {
                track_ahead_free_granted();
            }
        }
    }
    update_dialog_step(command, value);
}
std::map<string, string> persistent_commands;
void dmi_joined(BasePlatform::BusSocket::ClientId client)
{
    dmi_socket->on_peer_join().then(dmi_joined).detach();

    if (client.tid == BasePlatform::BusSocket::ClientId::fourcc("DMI_"))
        for (const auto &entry : persistent_commands)
            dmi_socket->send_to(client.uid, entry.first+"("+entry.second+")");
}
void dmi_data_received(std::pair<BasePlatform::BusSocket::ClientId, std::string> &&data)
{
    parse_command(std::move(data.second));
    dmi_socket->receive().then(dmi_data_received).detach();
}
void sim_write_line(const std::string &str);
bool sendtoor=false;
int64_t lastor;
void set_persistent_command(string command, string value)
{
    send_command(command, value);
    persistent_commands.insert(std::make_pair(command, value));
}
void send_command(string command, string value)
{
    if (dmi_socket)
        dmi_socket->broadcast(BasePlatform::BusSocket::ClientId::fourcc("DMI_"), command+"("+value+")");
    if (sendtoor)
        sim_write_line("noretain(etcs::dmi::command="+command+"("+value+"))");
}
double calc_ceiling_limit();
struct speed_element
{
    double Distance;
    double Speed;
};
struct gradient_element
{
    double Distance;
    int Gradient;
};
void to_json(json&j, const speed_element &e)
{
    j["DistanceToTrainM"] = e.Distance;
    j["TargetSpeedMpS"] = e.Speed;
}
void to_json(json&j, const gradient_element &e)
{
    j["DistanceToTrainM"] = e.Distance;
    j["GradientPerMille"] = e.Gradient;
}
void to_json(json&j, const PlanningTrackCondition &e)
{
    j["DistanceToTrainM"] = e.DistanceToTrainM;
    j["YellowColour"] = e.YellowColour;
    j["Type"] = e.Type;
    j["TractionSystem"] = e.TractionSystem;
}
void to_json(json&j, const text_message &t)
{
    j["Text"] = t.text;
    j["FirstGroup"] = t.firstGroup;
    j["Acknowledge"] = t.ack;
}
void dmi_update_func()
{
    sendtoor = get_milliseconds() - lastor > 250;
    if (sendtoor) lastor = get_milliseconds();
    json j;
    j["AllowedSpeedMpS"] = V_perm;
    j["InterventionSpeedMpS"] = V_sbi;
    j["TargetSpeedMpS"] = V_target;
    j["TargetDistanceM"] = D_target;
    j["SpeedMpS"] = V_est;
    j["ReleaseSpeedMpS"] = V_release;
    j["CurrentMonitoringStatus"] = (int)monitoring;
    j["CurrentSupervisionStatus"] = (int)supervision;
    j["CurrentMode"] = (int)mode;
    j["CurrentLevel"] = (int)level;
    if (level == Level::NTC)
        j["CurrentNTC"] = nid_ntc;
    j["TimeToPermittedS"] = TTP;
    j["TimeToIndicationS"] = TTI;
    if (mode_acknowledgeable) j["ModeAcknowledgement"] = (int)mode_to_ack;
    else j["ModeAcknowledgement"] = nullptr;
    if (ongoing_transition || level_acknowledgeable) {
        j["LevelTransition"]["Acknowledge"] = level_acknowledgeable;
        j["LevelTransition"]["Level"] = (int)level_to_ack;
        if (level_to_ack == Level::NTC)
            j["LevelTransition"]["NTC"] = ntc_to_ack;
    } else {
        j["LevelTransition"] = nullptr;
    }
    j["OverrideActive"] = overrideProcedure;
    j["RadioStatus"] = (int)radio_status_driver;
    j["BrakeCommanded"] = EB_command || SB_command;
    j["BrakeAcknowledge"] = brake_acknowledgeable;
    if (valid_geo_reference) j["GeographicalPositionKM"] = valid_geo_reference->get_position(d_estfront);
    else j["GeographicalPositionKM"]=nullptr;
    j["TextMessages"] = messages;
    j["DisplayTAF"] = start_display_taf && !stop_display_taf;
    if (display_lssma) j["LSSMA"] = lssma;
    if (mode == Mode::FS || mode == Mode::OS)
    {
        std::vector<speed_element> speeds;
        double v = calc_ceiling_limit();
        speeds.push_back({0,v});
        std::map<::distance,double> MRSP = get_MRSP();
        extern const target* indication_target;
        extern double indication_distance;
        double last_distance = MA ? MA->get_abs_end()-d_minsafefront(MA->get_abs_end()) : 0;
        const std::list<target> &targets = get_supervised_targets();
        for (const target &t : targets)
        {
            distance td = t.get_target_position();
            double d = td - (t.is_EBD_based ? d_maxsafefront(td) : d_estfront);
            if (t.get_target_speed() == 0 && d<last_distance)
                last_distance = d;
        }
        j["IndicationMarkerTarget"] = nullptr;
        j["IndicationMarkerDistanceM"] = nullptr;
        for (auto it=MRSP.begin(); it!=MRSP.end(); ++it) {
            distance dist = it->first;
            float safedist = dist-d_maxsafefront(dist);
            if (safedist < 0)
                continue;
            if (safedist > last_distance + 1)
                break;
            if (indication_target != nullptr && indication_target->get_target_position() == dist && indication_target->get_target_speed() == it->second && indication_target->type == target_class::MRSP && monitoring == CSM) {
                j["IndicationMarkerTarget"]["TargetSpeedMpS"] = indication_target->get_target_speed();
                j["IndicationMarkerTarget"]["DistanceToTrainM"] = safedist;
                j["IndicationMarkerDistanceM"] = indication_distance;
            }
            speeds.push_back({safedist, it->second});
        }
        if (SvL && *SvL-d_maxsafefront(*SvL) <= last_distance + 1) {
            if (monitoring == CSM && indication_target != nullptr && (indication_target->type == target_class::SvL || indication_target->type == target_class::EoA)){
                j["IndicationMarkerTarget"]["TargetSpeedMpS"] = 0;
                j["IndicationMarkerTarget"]["DistanceToTrainM"] = *SvL-d_maxsafefront(*SvL);
                j["IndicationMarkerDistanceM"] = indication_distance;
            }
            speeds.push_back({*SvL-d_maxsafefront(*SvL), 0});
            last_distance = *SvL-d_maxsafefront(*SvL);
        }
        else if (EoA && *EoA-d_estfront <= last_distance + 1) {
            if (monitoring == CSM && indication_target != nullptr && (indication_target->type == target_class::SvL || indication_target->type == target_class::EoA)) {
                j["IndicationMarkerTarget"]["TargetSpeedMpS"] = 0;
                j["IndicationMarkerTarget"]["DistanceToTrainM"] = *EoA-d_estfront;
                j["IndicationMarkerDistanceM"] = indication_distance;
            }
            speeds.push_back({*EoA-d_estfront, 0});
            last_distance = *EoA-d_estfront;
        }
        if (LoA && LoA->first-d_maxsafefront(LoA->first) <= last_distance + 1) {
            if (monitoring == CSM && indication_target != nullptr && (indication_target->type == target_class::LoA)) {
                j["IndicationMarkerTarget"]["TargetSpeedMpS"] = LoA->second;
                j["IndicationMarkerTarget"]["DistanceToTrainM"] = LoA->first-d_maxsafefront(LoA->first);
                j["IndicationMarkerDistanceM"] = indication_distance;
            }
            speeds.push_back({LoA->first-d_maxsafefront(LoA->first), LoA->second});
            last_distance = LoA->first-d_maxsafefront(LoA->first);
        }
        j["SpeedTargets"] = speeds;
        std::map<::distance,double> gradient = get_gradient();
        std::vector<gradient_element> grad;
        grad.push_back({0, (int)((--gradient.upper_bound(d_estfront))->second*1000)});
        for (auto it=gradient.upper_bound(d_estfront); it!=gradient.end(); ++it) {
            float dist = it->first-d_estfront;
            if (it == --gradient.end() || dist >= last_distance + 1)
                break;
            grad.push_back({dist,(int)(it->second*1000)});
        }
        grad.push_back({std::max(last_distance, 0.0),0});
        j["GradientProfile"] = grad;
        std::vector<PlanningTrackCondition> objs;
        for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
            track_condition *tc = it->get();
            double start = tc->announce_distance;
            double end = tc->get_end_distance_to_train();
            tc->start_symbol.DistanceToTrainM = start;
            tc->end_symbol.DistanceToTrainM = end;
            if (tc->start_symbol.Type != TrackConditionType_DMI::None && start > 0 && start <= last_distance + 1) {
                objs.push_back(tc->start_symbol);
            }
            if (tc->end_symbol.Type != TrackConditionType_DMI::None && end > 0 && end <= last_distance + 1) {
                objs.push_back(tc->end_symbol);
            }
        }
        std::sort(objs.begin(), objs.end(), [](PlanningTrackCondition x, PlanningTrackCondition y) {return x.DistanceToTrainM < y.DistanceToTrainM;});
        j["PlanningTrackConditions"] = objs;
        std::set<int> active_symbols;
        for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
            track_condition *tc = it->get();
            if (tc->active_symbol != -1 && tc->order)
                active_symbols.insert(tc->active_symbol);
            else if (tc->announcement_symbol != -1 && tc->announce)
                active_symbols.insert(tc->announcement_symbol);
            else if (tc->end_active_symbol != -1 && tc->display_end) {
                active_symbols.insert(tc->end_active_symbol);
            }
        }
        extern bool inform_lx;
        if (inform_lx) active_symbols.insert(100);
        j["ActiveTrackConditions"] = active_symbols;
    }
    else
    {
        j["PlanningTrackConditions"] = "[]"_json;
        j["GradientProfile"] = "[]"_json;
        j["SpeedTargets"] = "[]"_json;
        j["ActiveTrackConditions"] = "[]"_json;
    }
    json j2;
    j2["Status"] = j;
    j2["ActiveWindow"] = active_window_dmi;
    send_command("json", j2.dump());
    send_command("setVset", to_string(V_set * 3.6));
    /*
    send_command("setGeoPosition", valid_geo_reference ? to_string(valid_geo_reference->get_position(d_estfront)) : "-1");
    auto m = mode;
    */
    platform->delay(100).then(dmi_update_func).detach();
}
