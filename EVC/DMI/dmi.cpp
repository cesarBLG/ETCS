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
#include "../Supervision/supervision.h"
#include <mutex>
#include <iostream>
#include <chrono>
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
extern MonitoringStatus monitoring;
extern SupervisionStatus supervision;
#ifdef WIN32
int write(int fd, const char *buff, size_t size)
{
    return send(fd,buff,size,0);
}
#endif
void dmi_comm()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5010);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    write(fd, "setMode(FS);\n", 13);
    write(fd, "setLevel(1);\n", 13);
    for (;;) {
        unique_lock<mutex> lck(loop_mtx);
        string s = "setMonitor(xSM);\n";
        s[11] = monitoring == TSM ? 'T' : (monitoring == RSM ? 'R' : 'C');
        write(fd, s.c_str(), s.size());
        s = "setSupervision(";
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
        s+=");\n";
        write(fd, s.c_str(), s.size());
        s = "setVperm("+to_string(V_perm*3.6)+");\n";
        write(fd, s.c_str(), s.size());
        s = "setVtarget("+to_string(V_target*3.6)+");\n";
        write(fd, s.c_str(), s.size());
        s = "setVest("+to_string(V_est*3.6)+");\n";
        write(fd, s.c_str(), s.size());
        s = "setVsbi("+to_string(V_sbi*3.6)+");\n";
        write(fd, s.c_str(), s.size());
        s = "setDtarget("+to_string(D_target)+");\n";
        write(fd, s.c_str(), s.size());
        s = "setVrelease("+to_string(V_release*3.6)+");\n";
        write(fd, s.c_str(), s.size());
        s = "setEB(";
        s += (EB ? "true" : "false");
        s += ");\n";
        write(fd, s.c_str(), s.size());
        lck.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
