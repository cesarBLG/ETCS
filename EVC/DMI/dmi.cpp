/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cstdlib>
#include <string>
#include <list>
#include <algorithm>
#include <map>
#include "../language/language.h"
#include "../Supervision/supervision.h"
#include "../Supervision/supervision_targets.h"
#include "../Supervision/targets.h"
#include "../Supervision/train_data.h"
#include "../Supervision/speed_profile.h"
#include "../Euroradio/session.h"
#include "../Position/distance.h"
#include "../Position/geographical.h"
#include "../Procedures/override.h"
#include "../Procedures/mode_transition.h"
#include "../Procedures/level_transition.h"
#include "../TrackConditions/track_condition.h"
#include "../TrainSubsystems/train_interface.h"
#include "../STM/stm.h"
#include "track_ahead_free.h"
#include "text_message.h"
#include "windows.h"
#include "acks.h"
#include "platform_runtime.h"

using std::map;
using std::set;
using std::list;
using std::string;
using std::to_string;
int dmi_pid;
std::unique_ptr<BasePlatform::BusSocket> dmi_socket;
void dmi_receive(BasePlatform::BusSocket::JoinNotification &&msg);
void dmi_receive(BasePlatform::BusSocket::LeaveNotification &&msg);
void dmi_receive(BasePlatform::BusSocket::Message &&msg);
void dmi_update_func();
void sim_write_line(const std::string &str);
void dmi_receive_handler(BasePlatform::BusSocket::ReceiveResult &&result)
{
    dmi_socket->receive().then(dmi_receive_handler).detach();
    std::visit([](auto&& arg){ dmi_receive(std::move(arg)); }, std::move(result));
}
void start_dmi()
{
    platform->delay(100).then(dmi_update_func).detach();
}
extern double V_set;
extern double TTI;
extern double TTP;
extern bool EB_command;
extern bool SB_command;
extern MonitoringStatus monitoring;
extern SupervisionStatus supervision;
bool messasge_when_driver_ack_level = false;
bool messasge_when_driver_ack_mode = false;
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
                if (messasge_when_driver_ack_mode) {
                    string targetMode = "";
                    switch (mode_to_ack) {
                        case Mode::FS:
                            targetMode = "FS";
                            break;
                        case Mode::LS:
                            targetMode = "LS";
                            break;
                        case Mode::OS:
                            targetMode = "OS";
                            break;
                        case Mode::SR:
                            targetMode = "SR";
                            break;
                        case Mode::SH:
                            targetMode = "SH";
                            break;
                        case Mode::UN:
                            targetMode = "UN";
                            break;
                        case Mode::PS:
                            targetMode = "PS";
                            break;
                        case Mode::SL:
                            targetMode = "SL";
                            break;
                        case Mode::SB:
                            targetMode = "SB";
                            break;
                        case Mode::TR:
                            targetMode = "TR";
                            break;
                        case Mode::PT:
                            targetMode = "PT";
                            break;
                        case Mode::SF:
                            targetMode = "SF";
                            break;
                        case Mode::IS:
                            targetMode = "IS";
                            break;
                        case Mode::NP:
                            targetMode = "NP";
                            break;
                        case Mode::NL:
                            targetMode = "NL";
                            break;
                        case Mode::SN:
                            targetMode = "SN";
                            break;
                        case Mode::RV:
                            targetMode = "RV";
                            break;
#if BASELINE == 4
                        case Mode::AD:
                            targetMode = "AD";
                            break;
                        case Mode::SM:
                            targetMode = "SM";
                            break;
#endif
                        default:
                            break;
                    }
                    int64_t time = get_milliseconds();
                    add_message(text_message(targetMode + get_text(" confirmed"), true, false, false, [time](text_message& t) { return time + 30000 < get_milliseconds(); }));
                }
            } else if (selection == "LevelAcknowledge") {
                level_acknowledgeable = false;
                level_acknowledged = true;
                if (messasge_when_driver_ack_level) {
                    string targetLevel = "";
                    switch (level_to_ack) {
                        case Level::N0:
                            targetLevel = get_text("Level 0");
                            break;
                        case Level::N1:
                            targetLevel = get_text("Level 1");
                            break;
                        case Level::N2:
                            targetLevel = get_text("Level 2");
                            break;
#if BASELINE < 4
                        case Level::N3:
                            targetLevel = get_text("Level 3");
                            break;
#endif
                        case Level::NTC:
                            targetLevel = get_ntc_name(ntc_to_ack);
                            break;
                        default:
                        case Level::Unknown:
                            break;
                    }
                    int64_t time = get_milliseconds();
                    add_message(text_message(targetLevel + get_text(" confirmed"), true, false, false, [time](text_message& t) { return time + 30000 < get_milliseconds(); }));
                }
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
    if (command == "stmData") {
        sim_write_line("noretain(stm::command_etcs="+value+")");
    }
    if (command == "ackButtonLight") {
        extern int ack_button_light;
        ack_button_light = stoi(value);
    }
    update_dialog_step(command, value);
}
std::map<string, string> persistent_commands;
void dmi_receive(BasePlatform::BusSocket::JoinNotification &&msg)
{
    if (msg.peer.tid == BasePlatform::BusSocket::PeerId::fourcc("DMI")) {
        for (const auto &entry : persistent_commands)
            dmi_socket->send_to(msg.peer.uid, entry.first+"("+entry.second+")");
        for (auto &t : messages) {
            dmi_socket->send_to(msg.peer.uid, "setMessage("+std::to_string(t.id)+","+std::to_string(t.text.size())+","+t.text+","+std::to_string(t.hour)+","+std::to_string(t.minute)+","+(t.firstGroup?"true,":"false,")+(t.ack?"true,":"false,")+std::to_string(t.reason)+")");
        }
    }
}
void dmi_receive(BasePlatform::BusSocket::LeaveNotification &&msg) {
}
void dmi_receive(BasePlatform::BusSocket::Message &&msg)
{
    parse_command(std::move(msg.data));
}
bool sendtoor=false;
int64_t lastor;
void set_persistent_command(string command, string value)
{
    send_command(command, value);
    persistent_commands.insert_or_assign(command, value);
}
void send_command(string command, string value)
{
    if (dmi_socket)
        dmi_socket->broadcast(BasePlatform::BusSocket::PeerId::fourcc("DMI"), command+"("+value+")");
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
    if ((!cab_active[0] && !cab_active[1]) || mode == Mode::NP || mode == Mode::PS || mode == Mode::SL) {
        dmi_socket = nullptr;
        platform->delay(100).then(dmi_update_func).detach();
        return;
    }
    if (!dmi_socket) {
        dmi_socket = platform->open_socket("evc_dmi", BasePlatform::BusSocket::PeerId::fourcc("EVC"));
        if (!dmi_socket)
            return;
        dmi_socket->receive().then(dmi_receive_handler).detach();
    }
    platform->delay(100).then(dmi_update_func).detach();
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
    j["CurrentLevel"] = (int)(level_valid ? level : Level::Unknown);
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
    j["AllowedAck"] = ack_allowed;
    json clock;
    clock["Hour"] = WallClockTime::hour;
    clock["Minute"] = WallClockTime::minute;
    clock["Second"] = WallClockTime::second;
    j["WallClockTime"] = clock;
    if (display_lssma) j["LSSMA"] = lssma;
    if (mode == Mode::FS || mode == Mode::OS)
    {
        std::vector<speed_element> speeds;
        double v = calc_ceiling_limit();
        speeds.push_back({0,v});
        auto &MRSP = get_MRSP();
        extern double indication_distance;
        double last_distance = MA ? MA->get_abs_end().min-d_minsafefront(MA->get_abs_end()) : 0;
        const std::list<std::shared_ptr<target>> &targets = get_supervised_targets();
        for (auto &t : targets)
        {
            relocable_dist_base td = t->get_target_position();
            double d = td - (t->is_EBD_based ? d_maxsafefront(td) : d_estfront);
            if (t->get_target_speed() == 0 && d<last_distance)
                last_distance = d;
        }
        j["IndicationMarkerTarget"] = nullptr;
        j["IndicationMarkerDistanceM"] = nullptr;
        for (auto it=MRSP.begin(); it!=MRSP.end(); ++it) {
            relocable_dist_base dist = it->first;
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
        if (SvL && SvL->max-d_maxsafefront(*SvL) <= last_distance + 1) {
            if (monitoring == CSM && indication_target != nullptr && (indication_target->type == target_class::SvL || indication_target->type == target_class::EoA)){
                j["IndicationMarkerTarget"]["TargetSpeedMpS"] = 0;
                j["IndicationMarkerTarget"]["DistanceToTrainM"] = SvL->max-d_maxsafefront(*SvL);
                j["IndicationMarkerDistanceM"] = indication_distance;
            }
            speeds.push_back({SvL->max-d_maxsafefront(*SvL), 0});
            last_distance = SvL->max-d_maxsafefront(*SvL);
        }
        else if (EoA && EoA->est-d_estfront <= last_distance + 1) {
            if (monitoring == CSM && indication_target != nullptr && (indication_target->type == target_class::SvL || indication_target->type == target_class::EoA)) {
                j["IndicationMarkerTarget"]["TargetSpeedMpS"] = 0;
                j["IndicationMarkerTarget"]["DistanceToTrainM"] = EoA->est-d_estfront;
                j["IndicationMarkerDistanceM"] = indication_distance;
            }
            speeds.push_back({EoA->est-d_estfront, 0});
            last_distance = EoA->est-d_estfront;
        }
        if (LoA && LoA->first.max-d_maxsafefront(LoA->first) <= last_distance + 1) {
            if (monitoring == CSM && indication_target != nullptr && (indication_target->type == target_class::LoA)) {
                j["IndicationMarkerTarget"]["TargetSpeedMpS"] = LoA->second;
                j["IndicationMarkerTarget"]["DistanceToTrainM"] = LoA->first.max-d_maxsafefront(LoA->first);
                j["IndicationMarkerDistanceM"] = indication_distance;
            }
            speeds.push_back({LoA->first.max-d_maxsafefront(LoA->first), LoA->second});
            last_distance = LoA->first.max-d_maxsafefront(LoA->first);
        }
        j["SpeedTargets"] = speeds;
        std::map<dist_base,double> gradient = get_gradient();
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
}
