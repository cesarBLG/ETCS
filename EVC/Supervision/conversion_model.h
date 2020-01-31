#pragma once
#include <map>
#include "acceleration.h"
void set_test_values();
void set_conversion_model();
acceleration get_A_gradient(std::map<distance, double> gradient);
extern double T_brake_emergency_cm0;
extern double T_brake_emergency_cmt;
extern double T_brake_service_cm0;
extern double T_brake_service_cmt;
extern acceleration A_brake_emergency;
extern acceleration A_brake_service;
extern acceleration A_brake_normal_service;
extern double Kt_int;
extern std::map<double, double> Kv_int;
extern std::map<double, double> Kr_int;
extern std::map<double, std::pair<double,double>> Kn;
extern std::map<double, double> Kwet_rst;
extern bool conversion_model_used;
double Kdry_rst(double V, double EBCL, distance d);