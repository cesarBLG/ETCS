#include "conversion_model.h"
#include "national_values.h"
#include "fixed_values.h"
#include "train_data.h"
#include <map>
#include <utility>
#include <cmath>
const double T_bs1_locked = 0;
const double T_bs2_locked = 2;
double T_bs;
double T_bs1;
double T_bs2;
double T_be;

std::map<distance, double> gradient;
double get_gradient(distance dist)
{
    return (--gradient.upper_bound(dist))->second;
}
void calculate_gradient()
{
    A_gradient = acceleration();
    A_gradient.accel = [=](double V, distance d) {
        double grad = get_gradient(d);
        const double g = 9.81;
        if (M_rotating_nom > 0)
            return g*grad/(1000+10*M_rotating_nom);
        else
            return g*grad/(1000+10*((grad>0) ? M_rotating_max : M_rotating_min));
    };
    for (auto it=gradient.begin(); it!=gradient.end(); ++it) {
        A_gradient.dist_step.insert(it->first);
    }
}

acceleration A_brake_emergency;
acceleration A_brake_safe;
acceleration A_safe;
acceleration A_brake_service;
acceleration A_expected;
acceleration A_brake_normal_service;
acceleration A_normal_service;
acceleration A_gradient;

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
void set_test_values()
{
    gradient[distance(-10000)] = 0;
    
    Kv_int[0] = 1;
    Kr_int[0] = 1;
    Kn[0] = {1,1};
    
    A_brake_emergency = conversion_acceleration(brake_percentage);
    
    A_brake_service = conversion_acceleration(std::min(brake_percentage, 135));
    
    A_brake_normal_service = acceleration();
    A_brake_normal_service.speed_step.insert(0);
    A_brake_normal_service.accel = [](double V, distance d) { return 0.6; };
    
    calculate_gradient();
    calculate();
}
int redadh;

void calculate()
{
    bool convmod = true;
    T_be = (convmod ? Kt_int : 1)*T_brake_emergency;
    T_bs = T_brake_service;
    if (convmod) {
        T_bs1 = T_bs2 = T_bs;
    } else {
    }
    
    if (convmod) {
        A_brake_safe = A_brake_emergency;
        A_brake_safe.accel = [=](double V, distance d) {
            return (--Kv_int.upper_bound(V))->second*(--Kr_int.upper_bound(L_TRAIN))->second*A_brake_emergency(V,d);
        };
        for (auto it=Kv_int.begin(); it!=Kv_int.end(); ++it)
            A_brake_safe.speed_step.insert(it->first);
    } else {
        A_brake_safe = A_brake_emergency;
        A_brake_safe.accel = [=](double V, distance d) {
            double wet = (--Kwet_rst.upper_bound(V))->second;
            return Kdry_rst(V,M_NVEBCL)*(wet+M_NVAVADH*(1-wet))*A_brake_emergency(V,d);
        };
    }
    
    A_safe = A_brake_safe + A_gradient;
    if (redadh>0) {
        double A_MAXREDADH = (redadh == 3 ? A_NVMAXREDADH3 : (redadh == 2 ? A_NVMAXREDADH2 : A_NVMAXREDADH1));
        A_safe.accel = [=](double V, distance d) { return std::min(A_brake_safe(V,d), A_MAXREDADH) + A_gradient(V,d); };
    }
    
    A_expected = A_brake_service + A_gradient;
    
    A_normal_service = A_brake_normal_service + A_gradient;
    A_normal_service.accel = [=](double V, distance d) {
        double grad = get_gradient(d);
        double kn = (grad > 0) ? (--Kn.upper_bound(V))->second.first : (--Kn.upper_bound(V))->second.second;
        return A_brake_normal_service(V,d) + A_gradient(V,d) - kn*grad/1000;
    };
    for (auto it=Kn.begin(); it!=Kn.end(); ++it)
        A_normal_service.speed_step.insert(it->first);
}
acceleration operator+(const acceleration a1, const acceleration a2)
{
    acceleration an;
    an.dist_step = a1.dist_step;
    an.speed_step = a1.speed_step;
    an.dist_step.insert(a2.dist_step.begin(), a2.dist_step.end());
    an.speed_step.insert(a2.speed_step.begin(), a2.speed_step.end());
    an.accel = [=](double V, distance d) {return a1.accel(V,d) + a2.accel(V,d);};
    return an;
}
