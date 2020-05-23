#ifndef _WIN32
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock2.h>
#endif
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <string>
#include <sstream>
#include <map>
#include "../Supervision/supervision.h"
#include "../Supervision/targets.h"
#include <mutex>
#include <iostream>
#include <chrono>
#include "../Supervision/train_data.h"
#include "../Supervision/speed_profile.h"
#include "../Position/distance.h"
#include "../Position/geographical.h"
#include "../Procedures/start.h"
#include "../Procedures/override.h"
#include "../Procedures/mode_transition.h"
#include "../Procedures/level_transition.h"
#include "../TrackConditions/track_condition.h"
#include "text_message.h"
using std::thread;
using std::mutex;
using std::unique_lock;
using std::map;
using std::set;
using std::string;
using std::to_string;
extern mutex loop_mtx;
int dmi_pid;
void dmi_comm();
void start_dmi()
{
#ifndef _WIN32
    printf("Starting Driver Machine Interface...\n");
    dmi_pid = fork();
    if(dmi_pid == 0)
    {
        chdir("../DMI");
        /*int fd = open("dmi.log.o", O_RDWR);
        dup2(fd, 2);*/
        execl("dmi", "dmi", nullptr);
    }
    sleep(1);
#else
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif
    thread thr(dmi_comm);
    thr.detach();
}
extern double V_target;
extern double V_perm;
extern double V_est;
extern double V_set;
extern double V_release;
extern double V_sbi;
extern double D_target;
extern bool EB;
extern bool SB;
extern MonitoringStatus monitoring;
extern SupervisionStatus supervision;
#ifdef WIN32
int write(int fd, const char *buff, size_t size)
{
    return send(fd,buff,size,0);
}
#endif
static int fd;
void parse_command(string str)
{
    int index = str.find_first_of('(');
    string command = str.substr(0, index);
    string value = str.substr(index+1, str.find_first_of(')')-index-1);
    unique_lock<mutex> lck(loop_mtx);
    if (command == "setTrainData") {
        special_train_data = value;
        validate_train_data();
    } else if (command == "setLtrain") {
        L_TRAIN = stof(value);
        special_train_data = "";
        validate_train_data();
    } else if (command == "setBrakePercentage") {
        brake_percentage = stof(value);
        special_train_data = "";
        validate_train_data();
    } else if (command == "setVtrain") {
        V_train = stof(value)/3.6;
        special_train_data = "";
        validate_train_data();
    } else if (command == "setCantDeficiency") {
        cant_deficiency = stoi(value);
        special_train_data = "";
        validate_train_data();
    } else if (command == "setTrainCategory") {
        special_train_data = "";
        int val = stoi(value);
        other_train_categories.clear();
        other_train_categories.insert(val);
        switch (val)
        {
            case 0:
                brake_position = FreightP;
                break;
            case 1:
                brake_position = FreightG;
                break;
            case 2:
                brake_position = PassengerP;
                break;
        }
        validate_train_data();
    } else if (command == "setLevel") {
        level = (Level)stoi(value);
    } else if (command == "startMission") {
        start_mission();
    } else if (command == "override") {
        start_override();
    } else if (command == "shunting") {
        if (V_est == 0 && (level==Level::N0 || level==Level::NTC || level==Level::N1)) {
            trigger_condition(5);
        }
    } else if (command == "messageAcked") {
        message_acked(stoi(value));
    } else if (command == "modeAcked") {
        mode_acknowledgeable = false;
        mode_acknowledged = true;
    } else if (command == "levelAcked") {
        level_acknowledgeable = false;
        level_acknowledged = true;
    }
}
void dmi_recv()
{
    char buff[500];
    string s;
    for (;;) {
        int count = recv(fd, buff, sizeof(buff)-1,0);
        buff[count] = 0;
        s+=buff;
        int end;
        while ((end=s.find_first_of(';'))!=string::npos) {
            int start = s.find_first_not_of("\n\r ;");
            string command = s.substr(start, end-start);
            parse_command(command);
            s = s.substr(end+1);
        }
    }

}
string lines = "";
void send_command(string command, string value)
{
    lines += command+"("+value+");\n";
}
double calc_ceiling_limit();
void dmi_comm()
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5010);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    thread reading(dmi_recv);
    reading.detach();
    for (;;) {
        unique_lock<mutex> lck(loop_mtx);
        send_command("setMode", Mode_str[(int)mode]);
        if (mode_acknowledgeable) {
            send_command("setModeTransition", Mode_str[(int)mode_to_ack]);
        } else {
            send_command("setModeTransition", "");
        }
        send_command("setLevel", to_string((int)level));
        if (ongoing_transition || level_acknowledgeable) {
            send_command("setLevelTransition", to_string((int)level_to_ack) + (level_acknowledgeable ? ",2" : ",1"));
        } else {
            send_command("setLevelTransition", "");
        }
        send_command("setMonitor",  monitoring == TSM ? "TSM" : (monitoring == RSM ? "RSM" : "CSM"));
        string s = "";
        switch(supervision)
        {
            case NoS:
                s += "NoS";
                break;
            case IndS:
                s += "IndS";
                break;
            case OvS:
                s += "OvS";
                break;
            case WaS:
                s += "WaS";
                break;
            case IntS:
                s += "IntS";
                break;
        }
        send_command("setSupervision", s);
        send_command("setVperm", to_string(V_perm*3.6));
        send_command("setVtarget", to_string(V_target*3.6));
        send_command("setVest", to_string(V_est*3.6));
        send_command("setVset", to_string(V_set*3.6));
        send_command("setVsbi", to_string(V_sbi*3.6));
        send_command("setVrelease", to_string(V_release*3.6));
        send_command("setDtarget", to_string(D_target));
        send_command("setEB", EB ? "true" : "false");
        send_command("setSB", SB ? "true" : "false");
        send_command("setOverride", overrideProcedure ? "true" : "false");
        send_command("setGeoPosition", valid_geo_reference ? to_string(valid_geo_reference->get_position(d_estfront)) : "-1");
        if (mode == Mode::FS) {
            string speeds="";
            double v = calc_ceiling_limit()*3.6;
            speeds += "0,"+to_string(v);
            std::map<::distance,double> MRSP = get_MRSP();
            extern target indication_target;
            extern double indication_distance;
            double last_distance = MA ? MA->get_end()-d_minsafefront(MA->get_end().get_reference()) : 0;
            for (const target &t : get_supervised_targets())
            {
                distance td = t.get_target_position();
                double d = td - (t.is_EBD_based ? d_maxsafefront(td.get_reference()) : d_estfront);
                if (t.get_target_speed() == 0 && d<last_distance)
                    last_distance = d;
            }
            for (auto it=MRSP.begin(); it!=MRSP.end(); ++it) {
                distance dist = it->first;
                float safedist = dist-d_maxsafefront(dist.get_reference());
                if (safedist < 0)
                    continue;
                if (safedist > last_distance + 1)
                    break;
                if (target(dist, it->second, target_class::MRSP) == indication_target && monitoring == CSM)
                    speeds+= ",im,"+to_string(indication_distance);
                speeds += ","+to_string(safedist)+","+to_string(it->second*3.6);
            }
            if (EoA && *EoA-d_estfront <= last_distance + 1) {
                if (monitoring == CSM && (indication_target.type == target_class::EoA || indication_target.type == target_class::SvL))
                    speeds+= ",im,"+to_string(indication_distance);
                speeds += ","+to_string(*EoA-d_estfront)+",0";
                last_distance = *EoA-d_estfront;
            }
            if (LoA && LoA->first-d_maxsafefront(LoA->first.get_reference()) <= last_distance + 1) {
                if (monitoring == CSM && (indication_target.type == target_class::LoA))
                    speeds+= ",im,"+to_string(indication_distance);
                speeds += ","+to_string(LoA->first-d_maxsafefront(LoA->first.get_reference()))+","+to_string(LoA->second*3.6);
                last_distance = LoA->first-d_maxsafefront(LoA->first.get_reference());
            }
            send_command("setPlanningSpeeds",speeds);
            std::map<::distance,double> gradient = get_gradient();
            string grad="0,"+to_string((--gradient.upper_bound(d_estfront))->second*100);
            for (auto it=gradient.upper_bound(d_estfront); it!=gradient.end(); ++it) {
                float dist = it->first-d_estfront;
                if (dist >= last_distance + 1)
                    break;
                grad+=","+to_string(dist)+","+to_string(it->second*100);
            }
            grad += ","+to_string(last_distance)+","+"0";
            send_command("setPlanningGradients",grad);

            std::vector<std::string> objs;
            for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
                track_condition *tc = it->get();
                double start = tc->get_distance_to_train();
                double end = tc->get_end_distance_to_train();
                if (tc->start_symbol != -1 && start > 0 && start <= last_distance + 1) {
                    objs.push_back(to_string(tc->start_symbol));
                    objs.push_back(to_string((int)start));
                }
                if (tc->end_symbol != -1 && end > 0 && end <= last_distance + 1) {
                    objs.push_back(to_string(tc->end_symbol));
                    objs.push_back(to_string((int)end));
                }
            }
            std::string ob = objs.empty() ? "": objs[0];
            for (int i=1; i<objs.size(); i++) {
                ob += ","+objs[i];
            }
            send_command("setPlanningObjects", ob);

            std::vector<string> active_symbols;
            for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
                track_condition *tc = it->get();
                if (tc->active_symbol != -1 && tc->order)
                    active_symbols.push_back(to_string(tc->active_symbol));
                else if (tc->announcement_symbol != -1 && tc->announce)
                    active_symbols.push_back(to_string(tc->announcement_symbol));
                else if (tc->end_active_symbol != -1 && tc->display_end) {
                    active_symbols.push_back(to_string(tc->end_active_symbol));
                }
            }
            std::string aob = active_symbols.empty() ? "" : active_symbols[0];
            for (int i=1; i<active_symbols.size(); i++) {
                aob += ","+active_symbols[i];
            }
            send_command("setActiveConditions", aob);
        } else {
            send_command("setPlanningObjects", "");
            send_command("setPlanningGradients", "");
            send_command("setPlanningSpeeds", "");
            send_command("setActiveConditions", "");
        }
        write(fd, lines.c_str(), lines.size());
        lines = "";
        lck.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
