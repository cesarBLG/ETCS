#include "asfa.h"
#include "../Supervision/supervision.h"
#include "../orts/common.h"
#include <string>
#include <mutex>
using namespace ORserver;
using namespace std;
extern ParameterManager manager;
bool AKT=false;
bool CON=true;
extern mutex loop_mtx;
extern mutex iface_mtx;
void initialize_asfa()
{
    std::unique_lock<mutex> lck(iface_mtx);
    Parameter *p = new Parameter("asfa::akt");
    p->GetValue = []() {
        unique_lock<mutex> lck(loop_mtx);
        return AKT ? "1" : "0";
    };
    manager.AddParameter(p);

    p = new Parameter("asfa::con");
    p->GetValue = []() {
        unique_lock<mutex> lck(loop_mtx);
        return CON ? "1" : "0";
    };
    manager.AddParameter(p);
}
void update_asfa()
{
    if (level == Level::N0 && mode == Mode::UN) {
        AKT = false;
        CON = true;
    } else if (level != Level::N0 && level != Level::Unknown && mode != Mode::UN) {
        AKT = true;
        CON = false;
    }
}