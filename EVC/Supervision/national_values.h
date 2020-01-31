#pragma once
#include <limits>
#include <map>
extern bool Q_NVDRIVER_ADHES;

extern double V_NVSHUNT;
extern double V_NVSTFF;
extern double V_NVONSIGHT;
extern double V_NVLIMSUPERV;
extern double V_NVUNFIT;
extern double V_NVREL;

extern double D_NVROLL;

extern bool Q_NVSBTSMPERM;
extern bool Q_NVEMRRLS;
extern bool Q_NVGUIPERM;
extern bool Q_NVSBFBPERM;
extern bool Q_NVINHSMICPERM;

extern double V_NVALLOWOVTRP;
extern double V_NVSUPOVTRP;

extern double D_NVOVTRP;

extern double T_NVOVTRP;

extern int M_NVDERUN;
extern int M_NVCONTACT;

extern double T_NVCONTACT;

extern double D_NVPOTRP;

extern double D_NVSTFF;

extern double Q_NVLOCACC;

extern int M_NVAVADH;

extern double M_NVEBCL;

struct kvint_pass_step
{
    double a;
    double b;
    double A_NVP12;
    double A_NVP23;
};
extern std::map<double,double> NV_KRINT;
extern std::map<double,double> NV_KVINT_freight;
extern std::map<double,kvint_pass_step> NV_KVINT_pass;
extern double M_NVKTINT;

extern double A_NVMAXREDADH1;
extern double A_NVMAXREDADH2;
extern double A_NVMAXREDADH3;
 
void setup_national_values();