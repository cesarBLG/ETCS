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
#ifndef _MOVEMENT_AUTHORITY_H
#define _MOVEMENT_AUTHORITY_H
#include <vector>
#include "../Packets/12.h"
#include "../Packets/15.h"
#include "../Supervision/speed_profile.h"
#include "../Supervision/targets.h"
#include "../optional.h"
#include "../Time/clock.h"
#include "mode_profile.h"
class timer
{
public:
    int64_t time;
    int64_t start_time;
    bool started;
    timer(int64_t time) : time(1000*time), started(false) {}
    void start()
    {
        start(get_milliseconds());
    }
    void start(int64_t start_time)
    {
        started = true;
        this->start_time = start_time;
    }
    bool triggered()
    {
        return started && start_time+time<get_milliseconds();
    }
};
class section_timer : public timer
{
    public:
    distance stoploc;
    section_timer(int64_t time, distance stoploc) : timer(time), stoploc(stoploc) {}
};
class end_timer : public timer
{
    public:
    distance startloc;
    end_timer(int64_t time, distance startloc) : timer(time), startloc(startloc) {}
};
struct ma_section
{
    double length;
    optional<section_timer> stimer;
};
struct danger_point
{
    double distance;
    double vrelease;
};
struct overlap
{
    optional<end_timer> ovtimer;
    double distance;
    double vrelease;
};
extern std::set<speed_restriction> signal_speeds;
class movement_authority
{
    double v_main;
    double v_ema;
    std::vector<ma_section> sections;
    optional<end_timer> endtimer;
    optional<danger_point> dp;
    optional<overlap> ol;
    optional<timer> loa_timer;
    distance start;
    int64_t time_stamp;
public:
    movement_authority(distance start, Level1_MA ma, int64_t first_balise_passed_time);
    movement_authority(distance start, Level2_3_MA ma, int64_t first_balise_passed_time);
    distance get_end()
    {
        distance end=start;
        for (int i=0; i<sections.size(); i++) {
            end += sections[i].length;
        }
        return end;
    }
    distance get_abs_end()
    {
        distance end = get_end();
        if (ol)
            end += ol->distance;
        else if (dp) 
            end += dp->distance;
        return end;
    }
    double get_v_main()
    {
        return v_main;
    }
    void update_timers();
    void shorten(distance d);
    void reposition(distance current, double new_length);
    friend void MA_infill(movement_authority ma);
    friend void replace_MA(movement_authority ma);
    friend void set_data();
    friend void set_signalling_restriction(movement_authority ma, bool infill);
};
extern optional<movement_authority> MA;
void calculate_SvL();
void replace_MA(movement_authority ma);
void delete_MA();
void set_signalling_restriction(movement_authority ma, bool infill);
#endif // _MOVEMENT_AUTHORITY_H
