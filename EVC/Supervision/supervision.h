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
const char Mode_str[][3] = {"SH","PT","TR","OS","SR","FS","NL","SB","RV","UN","IS","SF","NS","LS"};
extern Level level;
extern Mode mode;
extern double V_est;
extern double V_perm;
extern double V_sbi;
extern double D_target;
extern double V_ura;
extern double A_est1;
extern double A_est2;
extern double V_release;
extern double T_bs1;
extern double T_bs2;
extern double T_bs;
extern double T_be;
void update_supervision();
double calculate_V_release();
