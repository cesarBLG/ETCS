#include "interface.h"
#include <orts/client.h>
#include <orts/common.h>
#include <set>
#include <algorithm>
#include "../Supervision/distance.h"
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
extern distance d_maxsafefront;
extern distance d_minsafefront;
extern distance d_estfront;
POSIXclient *s_client;
std::set<Parameter*> parameters;
Parameter *GetParameter(string name)
{
    for(auto it=parameters.begin(); it!=parameters.end(); ++it) {
        if(**it==name) return *it;
    }
    Parameter *p = new Parameter(name);
    if(name=="distance") {
        p->SetValue = [](string val) {
            std::unique_lock<mutex> lck(loop_mtx);
            d_maxsafefront = distance(stof(val)*1.00001+10);
            d_minsafefront = distance(stof(val)*0.99999+10);
            d_estfront = distance(stof(val)+10);
        };
    } else if(name=="speed") {
        p->SetValue = [](string val) {
            std::unique_lock<mutex> lck(loop_mtx);
            V_est = stof(val)/3.6;
        };
    } else {
        delete p;
        return nullptr;
    }
    p->add_register(s_client);
    parameters.insert(p);
    return p;
}
void polling()
{
    threadwait *poller = new threadwait();
    s_client = new TCPclient("127.0.0.1", 5090, poller);
    s_client->WriteLine("register(speed)");
    s_client->WriteLine("register(distance)");
    while(s_client->connected) {
        int nfds = poller->poll(100);
        s_client->handle();
        string s = s_client->ReadLine();
        while(s!="") {
            ParseLine(s_client, s, GetParameter, [](client *c, Parameter *p) {parameters.erase(p);});
            s = s_client->ReadLine();
        }
        std::for_each(parameters.begin(), parameters.end(), [](Parameter* p){p->Send();});
    }
}
void start_or_iface()
{
    thread t(polling);
    t.detach();
}
