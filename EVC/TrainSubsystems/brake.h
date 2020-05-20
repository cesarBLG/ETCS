#pragma once
#include "../DMI/text_message.h"
struct brake_command_information
{
    text_message *msg;
    std::function<bool(brake_command_information &i)> revoke;
};
extern std::list<brake_command_information> brake_conditions;
void trigger_brake_reason(int reason);