#pragma once
#include "distance.h"
distance distance_curve(acceleration a, distance dref, double vref, double vel);
double speed_curve(acceleration a, distance dref, double vref, distance dist);
