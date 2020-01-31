#pragma once
#include "../Position/distance.h"
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
const char Mode_str[][3] = {"FS","LS","OS","SR","SH","UN","PS","SL","SB","TR","PT","SF","IS","NP","NL","SN","RV"};
extern Level level;
extern Mode mode;
extern double V_est;
extern double V_ura;
extern double A_est;
extern double V_perm;
extern double V_sbi;
extern double D_target;
extern double V_release;
extern double T_bs1;
extern double T_bs2;
void update_supervision();
double calculate_V_release();
