#include "interface.h"
#include <orts/client.h>
#include <orts/common.h>
#include <set>
#include <algorithm>
#include "../Supervision/distance.h"
#include "../Supervision/speed_profile.h"
#include "../MA/movement_authority.h"
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
extern bool EB;
extern bool SB;
double d_adjust=0;
double or_dist;
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
            or_dist = stof(val);
            double cval = or_dist-d_adjust;
            std::unique_lock<mutex> lck(loop_mtx);
            d_maxsafefront = distance(cval*1.00001);
            d_minsafefront = distance(cval*0.99999);
            d_estfront = distance(cval);
        };
    } else if(name=="speed") {
        p->SetValue = [](string val) {
            std::unique_lock<mutex> lck(loop_mtx);
            V_est = stof(val)/3.6;
        };
    } else if(name=="etcs_ssp") {
        p->SetValue = [](string val) {
            std::unique_lock<mutex> lck(loop_mtx);
            double prevadj = d_adjust;
            d_adjust = or_dist;
            double d_offset = or_dist-prevadj;
            d_maxsafefront = d_estfront;
            d_minsafefront = d_estfront;
            distance::update_distances(d_offset);
            std::vector<SSP_element> SSP;
            while(val.find_first_of(',')!=string::npos) {
                int pos = val.find_first_of(',');
                distance n1(std::stof(val.substr(0,pos)));
                val = val.substr(pos+1);
                pos = val.find_first_of(',');
                double n2(std::stof(val.substr(0,pos)));
                val = val.substr(pos+1);
                if (!SSP.empty() && (--SSP.end())->start==n1) {
                    n1 += 10000;
                }
                SSP.push_back(SSP_element(n1, n2, true));
                if (n2 < 0)
                    break;
            }
            update_SSP(SSP);
        };
    } else if(name=="etcs_ma") {
        p->SetValue = [](string val) {
            std::unique_lock<mutex> lck(loop_mtx);
            double prevadj = d_adjust;
            d_adjust = or_dist;
            double d_offset = or_dist-prevadj;
            d_maxsafefront = d_estfront;
            d_minsafefront = d_estfront;
            distance::update_distances(d_offset);
            if (MA != nullptr)
                delete MA;
            set_MA(new movement_authority(d_estfront, std::stof(val)));
        };
    } else if(name=="etcs_emergency") {
        p->GetValue = []() {
            std::unique_lock<mutex> lck(loop_mtx);
            return EB ? "true" : "false";
        };
    } else if(name=="etcs_fullbrake") {
        p->GetValue = []() {
            std::unique_lock<mutex> lck(loop_mtx);
            return SB ? "true" : "false";
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
    s_client->WriteLine("register(etcs_ssp)");
    s_client->WriteLine("register(etcs_ma)");
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
