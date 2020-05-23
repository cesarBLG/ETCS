/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "conversion_model.h"
#include "national_values.h"
#include "fixed_values.h"
#include "train_data.h"
#include "../Position/distance.h"
#include "acceleration.h"
#include <map>
#include <utility>
#include <cmath>
acceleration get_A_gradient(std::map<distance, double> gradient)
{
    acceleration A_gradient;
    A_gradient = acceleration();
    A_gradient.accel = [=](double V, distance d) {
        if (gradient.empty() || d-L_TRAIN<gradient.begin()->first)
            return 0.0;
        double grad = 50000;
        for (auto it=--gradient.upper_bound(d-L_TRAIN); it!=gradient.upper_bound(d); ++it) {
            grad = std::min(grad, it->second);
        }
        const double g = 9.81;
        if (M_rotating_nom > 0)
            return g*grad/(1000+10*M_rotating_nom);
        else
            return g*grad/(1000+10*((grad>0) ? M_rotating_max : M_rotating_min));
    };
    A_gradient.dist_step.insert(std::numeric_limits<double>::lowest());
    for (auto it=gradient.begin(); it!=gradient.end(); ++it) {
        A_gradient.dist_step.insert(it->first);
        A_gradient.dist_step.insert(it->first-L_TRAIN);
    }
    return A_gradient;
}
double T_brake_emergency_cm0;
double T_brake_emergency_cmt;
double T_brake_service_cm0;
double T_brake_service_cmt;
acceleration A_brake_emergency;
acceleration A_brake_service;
acceleration A_brake_normal_service;
std::map<distance,int> active_combination;
std::map<int,std::map<double, double>> A_brake_emergency_combination;
std::map<int,std::map<double, double>> A_brake_service_combination;
std::map<int,std::map<double,std::map<double, double>>> A_brake_normal_service_combination;
std::map<int,std::map<double,std::map<double,double>>> Kdry_rst_combination;
std::map<int,std::map<double, double>> Kwet_rst_combination;
std::map<int,double> T_brake_service_combination;
std::map<int,double> T_brake_emergency_combination;
acceleration get_A_brake_emergency()
{
    if (conversion_model_used)
        return A_brake_emergency;
    acceleration ac;
    for (auto it = active_combination.begin(); it!=active_combination.end(); ++it)
        ac.dist_step.insert(it->first);
    for (int i=0; i<16; i++) {
        for (auto it = A_brake_emergency_combination[i].begin(); it!=A_brake_emergency_combination[i].end(); ++it)
            ac.speed_step.insert(it->first);
    }
    ac.accel = [](double V, distance d)
    {
        int comb = (--active_combination.upper_bound(d))->second;
        return (--A_brake_emergency_combination[comb].upper_bound(V))->second;
    };
    return ac;
}
acceleration get_A_brake_service()
{
    if (conversion_model_used)
        return A_brake_service;
    acceleration ac;
    for (auto it = active_combination.begin(); it!=active_combination.end(); ++it)
        ac.dist_step.insert(it->first);
    for (int i=0; i<16; i++) {
        for (auto it = A_brake_service_combination[i].begin(); it!=A_brake_service_combination[i].end(); ++it)
            ac.speed_step.insert(it->first);
    }
    ac.accel = [](double V, distance d)
    {
        int comb = (--active_combination.upper_bound(d))->second;
        return (--A_brake_service_combination[comb].upper_bound(V))->second;
    };
    return ac;
}
acceleration get_A_brake_normal_service(acceleration service)
{
    acceleration ac;
    for (auto it = active_combination.begin(); it!=active_combination.end(); ++it)
        ac.dist_step.insert(it->first);
    for (auto it = A_brake_normal_service_combination[brake_position != PassengerP].begin(); it!=A_brake_normal_service_combination[brake_position].end(); ++it) {
        for (auto it2 = it->second.begin(); it2!=it->second.end(); ++it2)
            ac.speed_step.insert(it->first);
    }
    ac.accel = [service](double V, distance d)
    {
        return (--(--A_brake_normal_service_combination[brake_position != PassengerP].upper_bound(service(0,d)))->second.upper_bound(V))->second;
    };
    return ac;
}
double get_T_brake_emergency(distance d)
{
    return T_brake_emergency_combination[(--active_combination.upper_bound(d))->second];
}
double get_T_brake_service(distance d)
{
    return T_brake_service_combination[(--active_combination.upper_bound(d))->second];
}
double Kdry_rst(double V, double EBCL, distance d)
{
    return (--((--Kdry_rst_combination[(--active_combination.upper_bound(d))->second].upper_bound(V))->second.upper_bound(EBCL)))->second;
}
double Kwet_rst(double V, distance d)
{
    return (--Kwet_rst_combination[(--active_combination.upper_bound(d))->second].upper_bound(V))->second;
}
std::map<double,double> Kn[2];
void set_brake_model(json &traindata)
{
    json &brakes = traindata["brakes"];
    json &emergency = brakes["emergency"];
    for (auto it = emergency.begin(); it!=emergency.end(); ++it) {
        std::string valid = (*it)["validity"].get<std::string>();
        json &curves = (*it)["curves"];
        std::map<double,std::map<double,double>> dry;
        std::map<double,double> wet;
        std::map<double,double> accel;
        double build_time = (*it)["build_up_time"].get<double>();
        for (auto it2 = curves.begin(); it2!=curves.end(); ++it2) {
            json &step = *it2;
            double spd = step["speed"].get<double>()/3.6;
            accel[spd] = step["value"].get<double>();
            wet[spd] = step["kwet"].get<double>();
            json &dries = step["kdry"];
            for (auto it3 = dries.begin(); it3!=dries.end(); ++it3) {
                json &conf = *it3;
                double ebcl = conf["confidence"].get<double>();
                dry[spd][ebcl] = conf["value"].get<double>();
            }
        }
        for (int i=0; i<16; i++) {
            bool applies = true;
            for (int j=0; j<4; j++) {
                if (valid[j]!='.') {
                    int num = valid[j]-'0';
                    if (num != ((i>>j)&1)) {
                        applies = false;
                        break;
                    }
                }
            }
            if (applies) {
                A_brake_emergency_combination[i] = accel;
                T_brake_emergency_combination[i] = build_time;
                Kwet_rst_combination[i] = wet;
                Kdry_rst_combination[i] = dry;
            }
        }
    }
    json &service = brakes["service"];
    for (auto it = service.begin(); it!=service.end(); ++it) {
        std::string valid = (*it)["validity"].get<std::string>();
        json &curves = (*it)["curves"];
        std::map<double,double> accel;
        double build_time = (*it)["build_up_time"].get<double>();
        for (auto it2 = curves.begin(); it2!=curves.end(); ++it2) {
            json &step = *it2;
            double spd = step["speed"].get<double>()/3.6;
            accel[spd] = step["value"].get<double>();
        }
        for (int i=0; i<16; i++) {
            bool applies = true;
            for (int j=0; j<4; j++) {
                if (valid[j]!='.') {
                    int num = valid[j]-'0';
                    if (num != ((i>>j)&1)) {
                        applies = false;
                        break;
                    }
                }
            }
            if (applies) {
                A_brake_service_combination[i] = accel;
                T_brake_service_combination[i] = build_time;
            }
        }
    }
    json &normal = brakes["normal_service"];
    for (auto it = normal.begin(); it!=normal.end(); ++it) {
        std::string position = it.key();
        if (position == "kn+") {
            for (auto it2 = it->begin(); it2!=it->end(); ++it2) {
                Kn[0][(*it2)["speed"].get<double>()] = (*it2)["value"].get<double>();
            }
            continue;
        }
        if (position == "kn-") {
            for (auto it2 = it->begin(); it2!=it->end(); ++it2) {
                Kn[1][(*it2)["speed"].get<double>()] = (*it2)["value"].get<double>();
            }
            continue;
        }
        int p = position=="passenger" ? 0 : 1;
        for (auto it2 = it->begin(); it2!=it->end(); ++it2) {
            double sbaccel = stof(it2.key());
            json &curves = (*it2)["curves"];
            for (auto it3 = curves.begin(); it3!=curves.end(); ++it3) {
                json &step = *it3;
                double spd = step["speed"].get<double>()/3.6;
                A_brake_normal_service_combination[p][sbaccel][spd] = step["value"].get<double>();
            }
        }
    }
    active_combination[distance(std::numeric_limits<double>::lowest())] = 15;
}
std::map<double, double> Kv_int;
std::map<double, double> Kr_int;
double Kt_int;
acceleration conversion_acceleration(double lambda_0)
{
    double l1 = lambda_0;
    double l2 = l1*l1;
    double l3 = l1*l2;
    double V_lim = 16.85*std::pow(lambda_0,0.428);
    std::map<double,double> AD;
    AD[0] = 0.0075*lambda_0+0.076;
    double coef[5][4] = { {0.0663,4.72e-3,6.1e-5,6.3e-7},
                        {0.13,5.14e-3,-4.53e-6,2.73e-7},
                        {0.0479,5.81e-3,-6.76e-6,5.58e-8}, 
                        {0.048, 5.52e-3, -3.85e-6, 3e-8}, 
                        {0.0559, 5.06e-3, 1.66e-6, 3.23e-9}};
    double acel[5];
    for (int n=1; n<=5; n++) {
        acel[n-1] = coef[n-1][3]*l3+coef[n-1][2]*l2+coef[n-1][1]*l1+coef[n-1][0];
    }
    if (V_lim<100)
        AD[V_lim] = acel[0];
    if (V_lim > 100 && V_lim<120)
        AD[V_lim] = acel[1];
    if (V_lim<=100)
        AD[100] = acel[1];
    if (V_lim>120 && V_lim<150)
        AD[V_lim] = acel[2];
    if (V_lim<=120)
        AD[120] = acel[2];
    if (V_lim>150 && V_lim<180)
        AD[V_lim] = acel[3];
    if (V_lim<=150)
        AD[150] = acel[3];
    if (V_lim>180)
        AD[V_lim] = acel[4];
    if (V_lim<=180)
        AD[180] = acel[4];
    acceleration a_calculated;
    for (auto it=AD.begin(); it!=AD.end(); ++it) {
        a_calculated.speed_step.insert(it->first/3.6);
    }
    a_calculated.accel = [AD](double V, distance d) {
        return (--AD.upper_bound(V*3.6))->second;
    };
    return a_calculated;
}
inline double T_brake_basic(double L, double a, double b, double c)
{
    return a+L/100*(b+c*(L/100));
}
void convmodel_basic_accel(int lambda, acceleration &A_brake_emergency, acceleration &A_brake_service)
{    
    A_brake_emergency = conversion_acceleration(lambda);
    
    A_brake_service = conversion_acceleration(std::min(lambda, 135));

    A_ebmax=0;

    for (double spd : A_brake_emergency.speed_step) {
        A_ebmax = std::max(A_ebmax, A_brake_emergency.accel(spd, distance(0)));
    }
}
void convmodel_brake_build_up(brake_position_types brake_position, double L_TRAIN, double &T_brake_emergency_cm0, double &T_brake_emergency_cmt, double &T_brake_service_cm0, double &T_brake_service_cmt)
{
    double T_brake_basic_eb;
    if (brake_position == PassengerP) 
        T_brake_basic_eb = T_brake_basic(2.3,0,0.17, std::max(400.0, L_TRAIN));
    else if (brake_position == FreightP)
        T_brake_basic_eb = (L_TRAIN<=900) ? T_brake_basic(2.3,0,0.17, std::max(400.0, L_TRAIN)) : T_brake_basic(-0.5,1.6,0.03, L_TRAIN);
    else
        T_brake_basic_eb = (L_TRAIN<=900) ? T_brake_basic(12,0,0.05, L_TRAIN) : T_brake_basic(-0.4,1.6,0.03, L_TRAIN);

    T_brake_emergency_cm0 = T_brake_basic_eb;
    double Ct = brake_position == FreightG ? 0.16 : (brake_position == FreightP ? 0.20 : 0.20);
    double kto = 1 + Ct;
    T_brake_emergency_cmt = kto*T_brake_basic_eb;

    double T_brake_basic_sb;
    if (brake_position == PassengerP) 
        T_brake_basic_sb = T_brake_basic(3,1.5,0.1, L_TRAIN);
    else if (brake_position == FreightP)
        T_brake_basic_sb = (L_TRAIN<=900) ? T_brake_basic(3,2.77,0, L_TRAIN) : T_brake_basic(10.5,0.32,0.18, L_TRAIN);
    else
        T_brake_basic_sb = (L_TRAIN<=900) ? T_brake_basic(3,2.77,0, std::max(400.0, L_TRAIN)) : T_brake_basic(10.5,0.32,0.18, L_TRAIN);

    T_brake_service_cm0 = T_brake_basic_sb;

    T_brake_service_cmt = kto*T_brake_basic_sb;
}
void set_conversion_correction_values()
{
    if (brake_position == PassengerP) {
        for (auto it=NV_KVINT_pass.begin(); it!=NV_KVINT_pass.end(); ++it) {
            double a = it->second.a;
            double b = it->second.b;
            double A_NVP12 = it->second.A_NVP12;
            double A_NVP23 = it->second.A_NVP23;
            double val;
            if (A_ebmax <= A_NVP12)
                val = a;
            else if (A_ebmax >= A_NVP23)
                val = b;
            else
                val = a + (A_ebmax - A_NVP12)/(A_NVP23-A_NVP12)*(b-a);
            Kv_int[it->first] = val;
        }
    } else {
        Kv_int = NV_KVINT_freight;
    }
    Kr_int = NV_KRINT;
    Kt_int = M_NVKTINT;
}
bool conversion_model_used = false;
void set_conversion_model()
{
    if (brake_percentage >= 30 && brake_percentage <= 250 && V_train <= 200/3.6) {
        conversion_model_used = true;
        convmodel_basic_accel(brake_percentage, A_brake_emergency, A_brake_service);
        convmodel_brake_build_up(brake_position, L_TRAIN, T_brake_emergency_cmt, T_brake_emergency_cm0, T_brake_service_cmt, T_brake_service_cm0);
        set_conversion_correction_values();
    }
}
