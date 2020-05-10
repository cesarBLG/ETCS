#include "interface.h"
#include <orts/client.h>
#include <orts/common.h>
#include <set>
#include <bitset>
#include <algorithm>
#include "../Position/distance.h"
#include "../Supervision/speed_profile.h"
#include "../MA/movement_authority.h"
#include "../Position/linking.h"
#include "../Packets/messages.h"
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
extern distance d_estfront;
extern bool EB;
extern bool SB;
extern bool TCO;
double or_dist;
POSIXclient *s_client;
ParameterManager manager;
mutex iface_mtx;
void SetParameters()
{
    std::unique_lock<mutex> lck(iface_mtx);
    Parameter *p = new Parameter("distance");
    p->SetValue = [](string val) {
        or_dist = stof(val);
        std::unique_lock<mutex> lck(loop_mtx);
        odometer_value = or_dist;
    };
    manager.AddParameter(p);

    p = new Parameter("speed");
    p->SetValue = [](string val) {
        std::unique_lock<mutex> lck(loop_mtx);
        V_est = stof(val)/3.6;
        V_ura = 0.007*V_est;
        if (V_est < 0.2)
        {
            V_est = V_ura = 0;
        }
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::telegram");
    p->SetValue = [](string val) {
        std::unique_lock<mutex> lck(loop_mtx);
        std::vector<bool> message;
        for (int i=0; i<val.size(); i++) {
            message.push_back(val[i]=='1');
        }
        bit_read_temp r(message);
        eurobalise_telegram t(r);
        pending_telegrams.push_back(t);
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::emergency");
    p->GetValue = []() {
        std::unique_lock<mutex> lck(loop_mtx);
        return EB ? "true" : "false";
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::fullbrake");
    p->GetValue = []() {
        std::unique_lock<mutex> lck(loop_mtx);
        return SB ? "true" : "false";
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::tractioncutoff");
    p->GetValue = []() {
        std::unique_lock<mutex> lck(loop_mtx);
        return TCO ? "true" : "false";
    };
    manager.AddParameter(p);
    /*} else if(name=="speed") {
        p->SetValue = [](string val) {
            std::unique_lock<mutex> lck(loop_mtx);
            V_est = stof(val)/3.6;
            V_ura = 0.007*V_est;
        };
    } else if(name=="etcs::telegram") {
        p->SetValue = [](string val) {
            std::unique_lock<mutex> lck(loop_mtx);
            std::vector<bool> message;
            for (int i=0; i<val.size(); i++) {
                message.push_back(val[i]=='1');
            }
            bit_read_temp r(message);
            eurobalise_telegram t(r);
            pending_telegrams.push_back(t);
        };
    } else if(name=="etcs::ssp") {
        p->SetValue = [](string val) {
            std::unique_lock<mutex> lck(loop_mtx);
            distance start = distance(0,update_location_reference(1, d_estfront, true));
            std::map<distance, double> grad;
            grad[distance(0)] = 0;
            grad[distance(30000)] = 0;
            update_gradient(grad);
            InternationalSSP ISSP;
            ISSP.Q_SCALE.rawdata = Q_SCALE_t::m1;
            std::vector<SSP_element_packet> SSP;
            double dist=0;
            while(val.find_first_of(',')!=string::npos) {
                int pos = val.find_first_of(',');
                double n1 = std::stof(val.substr(0,pos));
                val = val.substr(pos+1);
                pos = val.find_first_of(',');
                double n2 = std::stof(val.substr(0,pos));
                val = val.substr(pos+1);
                SSP_element_packet pack;
                pack.N_ITER.rawdata = 0;

                pack.D_STATIC.rawdata = (n1-dist>0 || dist==0) ? (int)(n1-dist) : 10000;
                pack.V_STATIC.rawdata = (int)(n2*3.6/5);
                pack.Q_FRONT.rawdata = Q_FRONT_t::TrainLengthDelay;
                dist += n1;
                if (n2 < 0) {
                    pack.D_STATIC.rawdata = 32700;
                    pack.V_STATIC.rawdata = V_STATIC_t::EndOfProfile;
                    pack.Q_FRONT.rawdata = 0;
                    SSP.push_back(pack);
                    break;
                }
                SSP.push_back(pack);
            }
            ISSP.N_ITER.rawdata = SSP.size()-1;
            ISSP.element = SSP[0];
            ISSP.elements.insert(ISSP.elements.end(), ++SSP.begin(), SSP.end());
            update_SSP(get_SSP(start, ISSP));
        };
    } else if(name=="etcs::ma") {
        p->SetValue = [](string val) {
            std::unique_lock<mutex> lck(loop_mtx);
            distance start = distance(0,update_location_reference(1, d_estfront, true));
            Level1_MA ma;
            ma.Q_DIR.rawdata = Q_DIR_t::Nominal;
            ma.Q_SCALE.rawdata = Q_SCALE_t::m1;
            ma.N_ITER.rawdata = 0;
            ma.L_ENDSECTION.rawdata = std::stof(val);
            ma.Q_SECTIONTIMER.rawdata = 0;
            ma.Q_ENDTIMER.rawdata = 0;
            ma.Q_OVERLAP.rawdata = 0;
            ma.Q_DANGERPOINT.rawdata = 1;
            ma.D_DP.rawdata = 200;
            ma.V_RELEASEDP.rawdata = V_RELEASE_t::UseNationalValue;
            ma.V_MAIN.rawdata = 300/5;
            ma.V_EMA.rawdata = 0;
            replace_MA(movement_authority(start, ma));
        };
    } else if(name=="etcs::ma_infill") {
        p->SetValue = [](string val) {
            int com = val.find_first_of(',');
            string infill_start = val.substr(0,com);
            string len = val.substr(com+1);
            std::unique_lock<mutex> lck(loop_mtx);
            distance start = d_estfront;
            if (MA)
            {
                Level1_MA ma;
                ma.Q_DIR.rawdata = Q_DIR_t::Nominal;
                ma.Q_SCALE.rawdata = Q_SCALE_t::m1;
                ma.N_ITER.rawdata = 0;
                ma.L_ENDSECTION.rawdata = std::stof(len);
                ma.Q_SECTIONTIMER.rawdata = 0;
                ma.Q_ENDTIMER.rawdata = 0;
                ma.Q_OVERLAP.rawdata = 0;
                ma.Q_DANGERPOINT.rawdata = 1;
                ma.D_DP.rawdata = 200;
                ma.V_RELEASEDP.rawdata = V_RELEASE_t::UseNationalValue;
                ma.V_MAIN.rawdata = 300/5;
                ma.V_EMA.rawdata = 0;
                MA_infill(movement_authority(start+std::stof(infill_start), ma));
            }
        };
    } else {
        delete p;
        return nullptr;
    }
    p->add_register(s_client);
    parameters.insert(p);
    return p;*/
}
void polling()
{
    threadwait *poller = new threadwait();
    s_client = new TCPclient("127.0.0.1", 5090, poller);//TCPclient::connect_to_server(poller);
    s_client->WriteLine("register(speed)");
    s_client->WriteLine("register(distance)");
    /*s_client->WriteLine("register(etcs::ssp)");
    s_client->WriteLine("register(etcs::ma)");
    s_client->WriteLine("register(etcs::ma_infill)");*/
    s_client->WriteLine("register(etcs::telegram)");
    SetParameters();
    while(s_client->connected) {
        int nfds = poller->poll(100);
        s_client->handle();
        string s = s_client->ReadLine();
        std::unique_lock<mutex> lck(iface_mtx);
        while(s!="") {
            manager.ParseLine(s_client, s);
            s = s_client->ReadLine();
        }
        std::for_each(manager.parameters.begin(), manager.parameters.end(), [](Parameter* p){p->Send();});
    }
}
void start_or_iface()
{
    thread t(polling);
    t.detach();
}
