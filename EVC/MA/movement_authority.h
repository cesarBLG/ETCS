/*
 * European Train Control System
 * Copyright (C) 2019  César Benito <cesarbema2009@hotmail.com>
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

#ifndef _MOVEMENT_AUTHORITY_H
#define _MOVEMENT_AUTHORITY_H
#include <vector>
//#include "../parsed_packet.h"
#include "../Supervision/speed_profile.h"
#include "../Supervision/targets.h"
class timer
{
    float time;
public:
    timer(double time) : time(time)
    {
    }
    void start();
    void stop();
    void reset();
    void set(double time);
};
class section_timer : public timer
{
public:
    section_timer(double time, double stoploc) : timer(time) 
    {
        
    }
};
struct ma_section
{
    double length;
    section_timer *stimer;
};
struct danger_point
{
    double distance;
    double vrelease;
};
struct overlap
{
    double startdist;
    double time;
    double distance;
    double vrelease;
};
class movement_authority
{
    double v_main;
    double v_ema;
    std::vector<ma_section> sections;
    ma_section endsection;
    danger_point *dp;
    overlap *ol;
    distance start;
    distance end;
public:
    movement_authority(distance start, double length) : v_main(300/3.6), v_ema(0), start(start), end(start+length) //Test constructor
    {
        ol = nullptr;
        dp = new danger_point({200,-1});
    }
    //movement_authority(parsed_packet ma_packet);
    ~movement_authority();
    movement_authority(const movement_authority&) = delete;
    movement_authority &operator=(const movement_authority&) = delete;
    friend void set_MA(movement_authority *ma);
};
extern movement_authority *MA;
void set_MA(movement_authority *ma);
#endif // _MOVEMENT_AUTHORITY_H
