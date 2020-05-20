#pragma once
#include <map>
#include "acceleration.h"
#include "../Parser/nlohmann/json.hpp"
using json = nlohmann::json;
void set_brake_model(json &traindata);
void set_conversion_model();
acceleration get_A_gradient(std::map<distance, double> gradient);
extern double T_brake_emergency_cm0;
extern double T_brake_emergency_cmt;
extern double T_brake_service_cm0;
extern double T_brake_service_cmt;
double get_T_brake_emergency(distance d);
double get_T_brake_service(distance d);
acceleration get_A_brake_emergency();
acceleration get_A_brake_service();
acceleration get_A_brake_normal_service(acceleration A_brake_service);
extern double Kt_int;
extern std::map<double, double> Kv_int;
extern std::map<double, double> Kr_int;
extern std::map<double,double> Kn[2];
extern bool conversion_model_used;
double Kdry_rst(double V, double EBCL, distance d);
double Kwet_rst(double V, distance d);