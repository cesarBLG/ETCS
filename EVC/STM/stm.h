#pragma once
#include <map>
#include <vector>
#include "../Procedures/mode_transition.h"
#include "../Procedures/level_transition.h"
#include "../Supervision/supervision.h"
#include "../optional.h"
enum struct stm_state
{
    NP,
    PO,
    CO,
    DE,
    CS,
    HS,
    DA,
    FA
};
struct stm_state_order
{
    stm_state state;
    bool conditional;
};
struct stm_commands
{
    bool TCO;
    bool SB;
    bool EB_on_SB_failure;
    bool EB;
    bool open_circuit_breaker;
    bool lower_pantograph;
    bool close_air_intake;
    bool regenerative_brake_inhibition;
    bool magnetic_shoe_inhibition;
    bool eddy_emergency_brake_inhibition;
    bool eddy_service_brake_inhibition;
};
struct stm_forwarded_info
{
    int direction;
    int active_cab;
};
struct stm_object
{
    int nid_stm;
    stm_commands commands;
    stm_state state;
    optional<stm_state_order> last_order;
    int64_t last_order_time;
    int64_t last_national_trip;
    bool national_trip;
    bool isolated;
    bool control_request_EB;
    std::map<std::string, cond> conditions;
    stm_object();
    bool active()
    {
        if (last_order) {
            if (last_order->state == stm_state::DA)
                return true;
            if (last_order->state != stm_state::CS || !last_order->conditional)
                return false;
        }
        return state == stm_state::DA;
    }
    bool available()
    {
        return state == stm_state::CS || state == stm_state::HS || state == stm_state::DA;
    }
    void report_received(stm_state newstate);
    void report_override();
    void report_trip();
    void trigger_condition(std::string change);
    void request_state(stm_state req);
};
void stm_level_change(level_information newlevel, bool driver);
void stm_level_transition_received(level_transition_information info);
void send_failed_msg(stm_object *stm);
void assign_stm(int nid_ntc, bool driver);
void setup_stm_control();
void update_stm_control();
stm_object *get_stm(int nid_ntc);
std::string get_ntc_name(int nid_ntc);
extern std::map<int, stm_object*> installed_stms;
extern std::map<int, int> ntc_to_stm;
extern bool stm_control_EB;