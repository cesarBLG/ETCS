#include "conversion_model.h"
#include "national_values.h"
#include "fixed_values.h"
#include "train_data.h"
#include "distance.h"
#include "acceleration.h"
#include <map>
#include <utility>
#include <cmath>

acceleration get_A_gradient(std::map<distance, double> gradient)
{
    acceleration A_gradient;
    A_gradient = acceleration();
    A_gradient.accel = [=](double V, distance d) {
        double grad = (--gradient.upper_bound(d))->second;
        const double g = 9.81;
        if (M_rotating_nom > 0)
            return g*grad/(1000+10*M_rotating_nom);
        else
            return g*grad/(1000+10*((grad>0) ? M_rotating_max : M_rotating_min));
    };
    for (auto it=gradient.begin(); it!=gradient.end(); ++it) {
        A_gradient.dist_step.insert(it->first);
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

std::map<double, std::pair<double,double>> Kv_int_ab;
std::map<double, double> Kv_int;
std::map<double, double> Kr_int;
std::map<double, std::pair<double,double>> Kn;
std::map<double, double> Kwet_rst;
double Kdry_rst(double V, double EBCL)
{
    return 1;
}
double Kt_int;
void set_Kv_int_P()
{
    for (auto it=Kv_int_ab.begin(); it!=Kv_int_ab.end(); ++it)
    {
        double a = it->second.first;
        double b = it->second.second;
        double val;
        if (A_ebmax <= A_NVP12)
            val = a;
        else if (A_ebmax >= A_NVP23)
            val = b;
        else
            val = a + (A_ebmax - A_NVP12)/(A_NVP23-A_NVP12)*(b-a);
        Kv_int[it->first] = val;
    }
}
acceleration conversion_acceleration(double lambda_0)
{
    double l1 = lambda_0;
    double l2 = l1*l1;
    double l3 = l1*l3;
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
void set_test_values()
{    
    Kv_int[0] = M_NVKVINT;
    Kr_int[0] = M_NVKRINT;
    Kt_int = M_NVKTINT;
    Kn[0] = {1,1};
    
    A_brake_normal_service = acceleration();
    A_brake_normal_service.speed_step.insert(0);
    A_brake_normal_service.accel = [](double V, distance d) { return std::min(brake_percentage/200.0,0.7); };
}
void convmodel_basic_accel(int lambda, acceleration &A_brake_emergency, acceleration &A_brake_service)
{    
    A_brake_emergency = conversion_acceleration(lambda);
    
    A_brake_service = conversion_acceleration(std::min(lambda, 135));
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
bool convmod = false;
void set_conversion_model()
{
    if (brake_percentage >= 30 && brake_percentage <= 250 && V_train <= 200/3.6) {
        convmod = true;
        convmodel_basic_accel(brake_percentage, A_brake_emergency, A_brake_service);
        convmodel_brake_build_up(brake_position, L_TRAIN, T_brake_emergency_cmt, T_brake_emergency_cm0, T_brake_service_cmt, T_brake_service_cm0);
    }
}
