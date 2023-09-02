/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
void delete_information(Mode prev);