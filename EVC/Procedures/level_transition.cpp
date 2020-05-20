#include "level_transition.h"
#include "../Packets/messages.h"
#include "../Packets/information.h"
#include "../DMI/text_message.h"
#include "../TrainSubsystems/brake.h"
optional<level_transition_information> ongoing_transition;
Level level = Level::Unknown;
std::list<std::list<std::shared_ptr<etcs_information>>> transition_buffer;
bool level_acknowledgeable = false;
bool level_acknowledged = false;
Level level_to_ack;
void perform_transition()
{
    if (!ongoing_transition) return;
    level_transition_information lti = *ongoing_transition;
    ongoing_transition = {};
    if (level_to_ack == Level::NTC || level == Level::NTC || level_to_ack == Level::N0)
        level_acknowledgeable = !level_acknowledged;
    level = lti.leveldata.level;
    for (auto it=transition_buffer.begin(); it!=transition_buffer.end(); ++it) {
        for (auto it2 = it->begin(); it2!=it->end(); ++it2) {
            try_handle_information(*it2, *it);
        }
    }
    transition_buffer.clear();
    if (!level_acknowledged) {
        brake_conditions.push_back({nullptr, [](brake_command_information &i) {
            if (level_acknowledged || level_to_ack != level || !level_acknowledgeable)
                return true;
            return false;
        }});
    }
}
void update_level_status()
{
    if (!ongoing_transition) return;
    if (ongoing_transition->start<=d_estfront)
        perform_transition();
    else if (mode != Mode::SB && 
    (level_to_ack == Level::NTC || level == Level::NTC || level_to_ack == Level::N0) && 
    ongoing_transition->leveldata.startack < d_maxsafefront(ongoing_transition->leveldata.startack.get_reference()) && !level_acknowledged) {
        level_acknowledgeable = true;
    }
}
void level_transition_received(level_transition_information info)
{
    if (!ongoing_transition || ongoing_transition->leveldata.level != info.leveldata.level)
        level_acknowledged = false;
    level_acknowledgeable = false;
    transition_buffer.clear();
    transition_buffer.push_back({});
    if (info.leveldata.level == level) {
        ongoing_transition = {};
        return;
    }
    ongoing_transition = info;
    level_to_ack = ongoing_transition->leveldata.level;
    if (ongoing_transition->immediate)
        perform_transition();
}