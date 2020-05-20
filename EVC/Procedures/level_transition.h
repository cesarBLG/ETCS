#pragma once
#include "../Position/distance.h"
#include "../Supervision/supervision.h"
#include "../Packets/41.h"
#include "../Packets/47.h"
#include "../Packets/messages.h"
#include "../Packets/etcs_information.h"
#include "../optional.h"
#include <vector>
#include <list>
struct target_level_information
{
    distance startack;
    Level level;
    int ntc_id;
};
struct level_transition_information
{
    bool immediate;
    bool acknowledged = false;
    distance start;
    target_level_information leveldata;
    level_transition_information(LevelTransitionOrder o, distance ref)
    {
        if (o.D_LEVELTR == D_LEVELTR_t::Now)
            immediate = true;
        else
            immediate = false;
        start = ref+o.D_LEVELTR.get_value(o.Q_SCALE);
        std::vector<target_level_information> priorities;
        priorities.push_back({start-o.element.L_ACKLEVELTR.get_value(o.Q_SCALE), o.element.M_LEVELTR.get_level()});
        for (int i=0; i<o.elements.size(); i++) {
            priorities.push_back({start-o.elements[i].L_ACKLEVELTR.get_value(o.Q_SCALE), o.elements[i].M_LEVELTR.get_level(),  o.elements[i].NID_NTC});
        }
        for (int i=0; i<priorities.size(); i++) {
            if (priorities[i].level == Level::N0 || priorities[i].level == Level::N1) {
                leveldata = priorities[i];
                return;
            }
        }
        leveldata = priorities.back();
    }
    level_transition_information(ConditionalLevelTransitionOrder o, distance ref)
    {
        immediate = true;
        start = ref;
        std::vector<target_level_information> priorities;
        priorities.push_back({start, o.element.M_LEVELTR.get_level(), o.element.NID_NTC});
        for (int i=0; i<o.elements.size(); i++) {
            priorities.push_back({start, o.elements[i].M_LEVELTR.get_level(), o.elements[i].NID_NTC});
        }
        for (int i=0; i<priorities.size(); i++) {
            if (priorities[i].level == level) {
                leveldata = priorities[i];
                return;
            }
        }
        for (int i=0; i<priorities.size(); i++) {
            if (priorities[i].level == Level::N0 || priorities[i].level == Level::N1) {
                leveldata = priorities[i];
                return;
            }
        }
        leveldata = priorities.back();
    }
};
void update_level_status();
void level_transition_received(level_transition_information info);
extern optional<level_transition_information> ongoing_transition;
extern optional<distance> max_ack_distance;
extern std::list<std::list<std::shared_ptr<etcs_information>>> transition_buffer;
extern bool level_acknowledgeable;
extern bool level_acknowledged;
extern Level level_to_ack;