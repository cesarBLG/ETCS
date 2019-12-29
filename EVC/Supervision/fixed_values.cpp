#include "fixed_values.h"
#define TO_MPS(kph) kph/3.6
const double dV_ebi_min = TO_MPS(7.5);
const double dV_ebi_max = TO_MPS(15);
const double V_ebi_min = TO_MPS(110);
const double V_ebi_max = TO_MPS(210);

const double dV_sbi_min = TO_MPS(5.5);
const double dV_sbi_max = TO_MPS(10);
const double V_sbi_min = TO_MPS(110);
const double V_sbi_max = TO_MPS(210);

const double dV_warning_min = TO_MPS(4);
const double dV_warning_max = TO_MPS(5);
const double V_warning_min = TO_MPS(110);
const double V_warning_max = TO_MPS(140);

const double T_dispTTI = 14;
const double T_warning = 2;
const double T_driver = 4;

const double M_rotating_max = 0.15;
const double M_rotating_min = 0.02;

const double T_CYCRQSTD = 60;
const double T_ACK = 5;

const double D_Metal = 300;

const double D_keep_information = 300;