#include "curve_calc.h"
#include "speed_profile.h"
speed_restriction *calculate_PBD(distance start, distance end, distance PBD, bool service_brake, double gradient)
{
    double speed;

    return new speed_restriction(speed, start, end, false);
}