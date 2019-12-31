#ifdef unix
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
#include "../Supervision/supervision.h"
#include <mutex>
#include <iostream>
#include <chrono>
#include "../Supervision/train_data.h"
#include "../Supervision/speed_profile.h"
#include "../Supervision/distance.h"
#include "../Procedures/start.h"
using namespace std;
extern mutex loop_mtx;
int dmi_pid;
void dmi_comm();
void start_dmi()
{
#ifdef unix
    printf("Starting Driver Machine Interface...\n");
    dmi_pid = fork();
    if(dmi_pid == 0)
    {
        chdir("../DMI");
        int fd = open("dmi.log.o", O_WRONLY | O_CREAT);
        dup2(fd, 1);
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
    if (command == "setLtrain")
        L_TRAIN = stof(value);
    else if (command == "setBrakePercentage")
        brake_percentage = stof(value);
    else if (command == "setVtrain") {
        V_train = stof(value)/3.6;
        set_train_max_speed(V_train);
    } else if (command == "setLevel") {
        level = (Level)stoi(value);
    } else if (command == "startMission") {
        start_mission();
    }
}
void dmi_recv()
{
    char buff[500];
    string s;
    for (;;) {
        int count = read(fd, buff, sizeof(buff)-1);
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
void send_command(string command, string value)
{
    string txt = command+"("+value+");\n";
    write(fd, txt.c_str(), txt.size());
}
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
        send_command("setLevel", to_string((int)level));
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
        send_command("setVsbi", to_string(V_sbi*3.6));
        send_command("setVrelease", to_string(V_release*3.6));
        send_command("setDtarget", to_string(D_target));
        send_command("setEB", EB ? "true" : "false");
        send_command("setSB", SB ? "true" : "false");
        lck.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
