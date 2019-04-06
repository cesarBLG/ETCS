#ifndef _MONITOR_H
#define _MONITOR_H
enum MonitoringStatus
{
    CSM,
    TSM,
    RSM,
};
enum SupervisionStatus
{
    NoS,
    IndS,
    OvS,
    WaS,
    IntS
};
enum Level 
{
    N0,
    N1,
    N2,
    N3,
    NTC,
};
enum Mode
{
    SH,
    PT,
    TR,
    OS,
    SR,
    FS,
    NL,
    SB,
    RV,
    UN,
    IS,
    SF,
    NS,
    LS,
};
extern Level level;
extern int levelAck;
extern Mode mode;
extern bool modeAck;
extern MonitoringStatus monitoring;
extern SupervisionStatus supervision;
extern float Vtarget;
extern float Vperm;
extern float Vsbi;
extern float Vebi;
extern float Vrelease;
extern float Vest;
extern float Dtarg;
extern float TTI;
extern bool EB;
extern int trn;
extern unsigned long driverid;
void update();
void setSpeeds(float vtarg, float vperm, float vsbi, float vrelease, float vest, float dist);
void setMonitor(MonitoringStatus status);
void setSupervision(SupervisionStatus status);
#endif