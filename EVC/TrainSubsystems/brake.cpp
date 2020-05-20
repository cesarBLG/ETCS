#include <functional>
#include <list>
#include <algorithm>
#include "../Time/clock.h"
#include "brake.h"
extern bool SB;
extern bool EB;
bool SB_commanded;
bool EB_commanded;
std::list<brake_command_information> brake_conditions;
void trigger_brake_reason(int reason)
{
    if (reason == 0) {
        text_message msg("Balise read error", true, false, 2, [](text_message &msg){return false;});
        text_message *m = &add_message(msg);
        brake_conditions.push_back({m, [](brake_command_information &i) {
            if (V_est <= 0) {
                int64_t time = get_milliseconds();
                i.msg->end_condition = [time](text_message &m) {
                    return time+30000<get_milliseconds();
                };
                return true;
            }
            return false;
        }});
    } if (reason == 1) {
        extern bool standstill_acknowledged;
        extern bool standstill_applied;
        text_message msg("Standstill supervision", true, false, 2, [](text_message &msg){return !standstill_applied;});
        text_message *m = &add_message(msg);
        brake_conditions.push_back({m, [](brake_command_information &i) {
            if (V_est <= 0) {
                if (!i.msg->ack) {
                    i.msg->ack = true;
                    i.msg->shown = false;
                }
            }
            if (standstill_applied && i.msg->acknowledged) {
                standstill_acknowledged = true;
                i.msg->end_condition = [](text_message &msg){return true;};
            }
            return !standstill_applied;
        }});
    }
}
void handle_brake_command()
{
    for (auto it = brake_conditions.begin(); it!=brake_conditions.end(); ++it) {
        if (it->revoke(*it)) {
            auto next = it;
            ++next;
            brake_conditions.erase(it);
            it = --next;
        }
    }
    SB_commanded = !brake_conditions.empty();
    SB_commanded |= SB;
    EB_commanded = EB;
}