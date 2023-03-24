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
#pragma once
#include "../MA/movement_authority.h"
#include "../Supervision/supervision.h"
#include "../Supervision/speed_profile.h"
#include "../Supervision/targets.h"
#include "../Supervision/national_values.h"
#include <functional>
#include <vector>
#include <initializer_list>
#include <cmath>
extern bool mode_acknowledgeable;
extern bool mode_acknowledged;
extern Mode mode_to_ack;
extern int64_t last_mode_change;
extern bool desk_open;
class cond
{
    bool triggered;
    std::function<bool()> val;
    public:
    cond()
    {
        val = []() {return false;};
        triggered = false;
    }
    cond &operator=(const std::function<bool()> fun)
    {
        val = fun;
        return *this;
    }
    bool operator()()
    {
        if (triggered) {
            triggered = false;
            return true;
        }
        return val();
    }
    void trigger()
    {
        triggered = true;
    }
};
extern cond mode_conditions[];
struct mode_transition
{
    Mode from;
    Mode to;
    std::set<int> conditions;
    int priority;
    int happens()
    {
        int cond = -1;
        for (int c : conditions) {
            if (mode_conditions[c]())
                cond = c;
        }
        return cond;
    }
    mode_transition(Mode from, Mode to, std::set<int> conditionnum, int priority) : from(from), to(to), conditions(conditionnum), priority(priority)
    {

    }
};
extern optional<std::set<bg_id>> sh_balises;
extern optional<std::set<bg_id>> sr_balises;
void initialize_mode_transitions();
void update_mode_status();
void trigger_condition(int num);