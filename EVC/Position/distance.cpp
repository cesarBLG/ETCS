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
#include "distance.h"
#include "../Supervision/supervision.h"
#include "../Supervision/national_values.h"
#include "linking.h"
#include <limits>
#include <iostream>
std::set<_dist_base*> distance::distances;
void distance::update_distances(double expected, double estimated)
{
    for(_dist_base *d : distances) {
        
        if (d->ref == 0) {
            if (d->dist>std::numeric_limits<double>::lowest() && d->dist<std::numeric_limits<double>::max())
                d->dist -= expected;
        } else {
            d->ref -= estimated;
        }
    }
    update_odometer();
    V_release = calculate_V_release();
}
void distance::update_unlinked_reference(double newref)
{
    for(_dist_base *d : distances) {
        if (d->ref !=0 ) {
            double offset = newref-d->ref;
            d->ref = newref;
            if (d->dist>std::numeric_limits<double>::lowest() && d->dist<std::numeric_limits<double>::max())
                d->dist -= offset;
        }
    }
}
distance::distance()
{
    base = nullptr;
}
distance::distance(double val, double ref)
{
    base = new _dist_base(val, ref);
    distances.insert(base);
}
distance::distance(const distance &d)
{
    if (d.base != nullptr) {
        base = new _dist_base(*d.base);
        distances.insert(base);
    } else {
        base = nullptr;
    }
}
distance::distance(distance &&d)
{
    base = d.base;
    d.base = nullptr;
}
distance::~distance()
{
    if (base != nullptr) {
        distances.erase(base);
        delete base;
    }
}
distance &distance::operator = (const distance& d)
{
    if (base != nullptr) {
        distances.erase(base);
        delete base;
    }
    if (d.base != nullptr) {
        base = new _dist_base(*d.base);
        distances.insert(base);
    } else {
        base = nullptr;
    }
    return *this;
}
distance &distance::operator = (distance&& d)
{
    if (base != nullptr) {
        distances.erase(base);
        delete base;
    }
    base = d.base;
    d.base = nullptr;
    return *this;
}
distance d_maxsafefront(double reference)
{
    return distance((d_estfront.get()-reference)*1.01+(reference==0 ? Q_LOCACC_LRBG : Q_NVLOCACC), reference);
}
distance d_minsafefront(double reference)
{
    return distance((d_estfront.get()-reference)*0.99-(reference==0 ? Q_LOCACC_LRBG : Q_NVLOCACC), reference);
}
distance d_maxsafe(distance d, double reference)
{
    return distance((d.get()-reference)*1.01+(reference==0 ? Q_LOCACC_LRBG : Q_NVLOCACC), reference);
}
distance d_minsafe(distance d, double reference)
{
    return distance((d.get()-reference)*0.99-(reference==0 ? Q_LOCACC_LRBG : Q_NVLOCACC), reference);
}
distance d_estfront(0);
double odometer_value=0;
double odometer_reference;
void update_odometer()
{
    d_estfront = distance(odometer_value-odometer_reference);
}
void reset_odometer(double dist)
{
    odometer_reference += dist;
    update_odometer();
}