#include "distance.h"
#include "supervision.h"
#include <limits>
#include <iostream>
static std::set<double*> distances;
extern double V_est;
void update_train_position()
{
    d_maxsafefront += 1.001*0.05*V_est*3;
    d_minsafefront += 0.999*0.05*V_est*3;
    d_estfront += 1*0.05*V_est*3;
}
void distance::update_distances(double offset)
{
    for(double *d : distances) {
        double val = *d;
        if (val>std::numeric_limits<double>::lowest() && val<std::numeric_limits<double>::max())
            *d -= offset;
    }
    V_release = calculate_V_release();
}
distance::distance()
{
    dist = nullptr;
}
distance::distance(double val)
{
    dist = new double(val);
    distances.insert(dist);
}
distance::distance(const distance &d)
{
    if (d.dist != nullptr) {
        dist = new double(*d.dist);
        distances.insert(dist);
    } else {
        dist = nullptr;
    }
}
distance::distance(distance &&d)
{
    if (d.dist != nullptr) {
        dist = new double(*d.dist);
        distances.insert(dist);
    } else {
        dist = nullptr;
    }
}
distance::~distance()
{
    if (dist != nullptr) {
        distances.erase(dist);
        delete dist;
    }
}
distance &distance::operator = (const distance& d)
{
    if (dist != nullptr) {
        distances.erase(dist);
        delete dist;
    }
    if (d.dist != nullptr) {
        dist = new double(*d.dist);
        distances.insert(dist);
    } else {
        dist = nullptr;
    }
    return *this;
}
distance &distance::operator = (distance&& d)
{
    if (dist != nullptr) {
        distances.erase(dist);
        delete dist;
    }
    if (d.dist != nullptr) {
        dist = new double(*d.dist);
        distances.insert(dist);
    } else {
        dist = nullptr;
    }
    return *this;
}
distance d_maxsafefront(0);
distance d_minsafefront(0);
distance d_estfront(0);
