#include "monitor.h"
#include "distance/distance.h"
#include "sound/sound.h"
MonitoringStatus monitoring;
SupervisionStatus supervision;
Level level = N0;
int levelAck = 0;
Mode mode = SB;
bool modeAck = false;
float Vtarget;
float Vperm;
float Vsbi;
float Vebi;
float Vrelease;
float Vest;
float Dtarg;
float TTI = 20;
bool EB = false;
int trn;
unsigned long driverid;
void updateSupervision();
void update()
{
    updateSupervision();
}
void setSpeeds(float vtarg, float vperm, float vsbi, float vrelease, float vest, float dist)
{
    Vtarget = vtarg;
    Vperm = vperm;
    Vsbi = vsbi;
    Vrelease = vrelease;
    Vest = vest;
    Dtarg = dist;
}
void setMonitor(MonitoringStatus status)
{
    if(monitoring == CSM && (status == TSM || status == RSM)) playSinfo();
    monitoring = status;
}
void setSupervision(SupervisionStatus status)
{
    if(monitoring == TSM && supervision == IndS && status == OvS) playTooFast();
    if(status == WaS) playSwarning();
    else if(supervision == WaS) stopSwarning();
    supervision = status;
}
//Note: this should be computed in EVC
void updateSupervision()
{
    float P = Vperm;
    float SBI = Vsbi<Vperm ? Vperm + 5 : Vsbi;
    float W = (Vperm+Vsbi)/2;
    SupervisionStatus status = supervision;
    //if(supervision == IntS && !speedMonitorBrake) supervision = NoS;
    if(monitoring == CSM)
    {
        if(supervision != IntS)
        {
            if(Vest>SBI) status = IntS;
            else if(Vest>W) status = WaS;
            else if(Vest>P) status = OvS;
            else status = NoS;
        }
    }
    if(monitoring == TSM)
    {
        if(supervision != IntS)
        {
            if(Vest>SBI) status = IntS;
            else if(Vest>W) status = WaS;
            else if(Vest>P) status = OvS;
            else status = IndS;
        }
    }
    if(monitoring == RSM)
    {
        if(supervision != IntS)
        {
            if(Vest>Vrelease) status = IntS;
            else status = IndS;
        }
    }
    setSupervision(status);
}