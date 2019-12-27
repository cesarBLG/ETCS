#include "national_values.h"
#define TO_MPS(kph) kph/3.6
#include <limits>
bool Q_NVDRIVER_ADHES = false;

double V_NVSHUNT = TO_MPS(30);
double V_NVSTFF = TO_MPS(40);
double V_NVONSIGHT = TO_MPS(30);
double V_NVLIMSUPERV = TO_MPS(100);
double V_NVUNFIT = TO_MPS(100);
double V_NVREL = TO_MPS(40);

double D_NVROLL = 2;

bool Q_NVSBTSMPERM = true;
bool Q_NVEMRRLS = false;
bool Q_NVGUIPERM = false;
bool Q_NVSBFBPERM = false;
bool Q_NVINHSMICPERM = false;

double V_NVALLOWOVTRP = 0;
double V_NVSUPOVTRP = TO_MPS(30);

double D_NVOVTRP = 200;

double T_NVOVTRP = 60;

int M_NVDERUN = 0;
int M_NVCONTACT = 0;

double T_NVCONTACT = std::numeric_limits<double>::infinity();

double D_NVPOTRP = 200;

double D_NVSTFF = std::numeric_limits<double>::infinity();

double Q_NVLOCACC = 12;

int M_NVAVADH = 0;

double M_NVEBCL = 0.999999999;

double L_NVKRINT = 0;
double M_NVKRINT = 0.9;
double V_NVKVINT = 0;
double M_NVKVINT = 0.7;
double M_NVKTINT = 1.1;

double A_NVMAXREDADH1 = 1;
double A_NVMAXREDADH2 = 0.7;
double A_NVMAXREDADH3 = 0.7;

double A_NVP12 = 0;
double A_NVP23 = 0;
