#ifndef _MONITOR_H
#define _MONITOR_H
#include <string>
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
enum struct Level 
{
    N0,
    N1,
    N2,
    N3,
    NTC,
    Unknown
};
enum struct Mode
{
    FS,
    LS,
    OS,
    SR,
    SH,
    UN,
    PS,
    SL,
    SB,
    TR,
    PT,
    SF,
    IS,
    NP,
    NL,
    SN,
    RV
};
extern Level level;
extern bool level_valid;
extern int levelAck;
extern Level ackLevel;
extern Mode mode;
extern bool modeAck;
extern Mode ackMode;
extern MonitoringStatus monitoring;
extern SupervisionStatus supervision;
extern float Vtarget;
extern float Vperm;
extern float Vsbi;
extern float Vebi;
extern float Vrelease;
extern float Vest;
extern float Vset;
extern float Dtarg;
extern float TTI;
extern float TTP;
extern bool SB;
extern bool EB;
extern int trn;
extern bool trn_valid;
extern std::string driverid;
extern bool driverid_valid;
extern bool ovEOA;
extern bool train_data_valid;
extern int L_train;
extern bool ovEOA;
void update();
void setSpeeds(float vtarg, float vperm, float vsbi, float vrelease, float vest, float dist);
void setMonitor(MonitoringStatus status);
void setSupervision(SupervisionStatus status);
#endif