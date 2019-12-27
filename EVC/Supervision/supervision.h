#pragma once
#include "distance.h"
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
extern double V_est;
extern double V_ura;
extern double A_est1;
extern double A_est2;
void update_supervision();
