#include "acceleration.h"
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