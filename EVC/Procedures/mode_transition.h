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
extern std::map<int,cond> mode_conditions;
struct mode_transition
{
    Mode from;
    Mode to;
    std::vector<int> conditions;
    int priority;
    bool happens()
    {
        for (int c : conditions) {
            if (mode_conditions[c]())
                return true;
        }
        return false;
    }
    mode_transition(Mode from, Mode to, std::vector<int> conditionnum, int priority) : from(from), to(to), conditions(conditionnum), priority(priority)
    {

    }
};
void initialize_mode_transitions();
void update_mode_status();