#pragma once
#include <map>
#include <vector>
#include "../Procedures/mode_transition.h"
#include "../Procedures/level_transition.h"
#include "../Supervision/supervision.h"
#include "../optional.h"
#include "../Packets/STM/message.h"
#include "../Packets/STM/179.h"
#include "stm_state.h"
struct stm_connection
{
    int nid_stm;
    int version_X;
    int version_Y;
};
struct stm_tiu_function
{
    bool TCO=false;
    bool open_circuit_breaker=false;
    bool lower_pantograph=false;
    bool close_air_intake=false;
    bool regenerative_brake_inhibition=false;
    bool magnetic_shoe_inhibition=false;
    bool eddy_emergency_brake_inhibition=false;
    bool eddy_service_brake_inhibition=false;
    int direction;
    int active_cab;
};
struct stm_biu_function
{
    bool SB=false;
    bool EB_on_SB_failure=false;
    bool EB=false;
};
struct stm_specific_data
{
    int id;
    std::string caption;
    std::string value;
    std::vector<std::string> keys;
    stm_specific_data() = default;
    stm_specific_data(const STMSpecificDataField &field)
    {
        id = field.NID_DATA;
        caption = X_CAPTION_t::getUTF8(field.X_CAPTION);
        value = X_VALUE_t::getUTF8(field.value.X_VALUE);
        for (auto &v : field.values) {
            keys.push_back(X_VALUE_t::getUTF8(v.X_VALUE));
        }
    }
};
struct stm_object
{
    int nid_stm;
    stm_tiu_function tiu_function;
    stm_biu_function biu_function;
    stm_state state;
    optional<stm_state> last_order;
    int64_t last_order_time;
    int64_t last_national_trip;
    int specific_data_need;
    int64_t data_entry_timer;
    enum struct data_entry_state
    {
        Inactive,
        Start,
        Active,
        Driver,
        DataSent,
    } data_entry;
    bool national_trip;
    bool isolated;
    bool control_request_EB;
    std::map<std::string, cond> conditions;
    std::vector<stm_specific_data> specific_data;
    stm_connection *control_connection;
    stm_connection *tiu_connection;
    stm_connection *biu_connection;
    stm_object();
    bool active()
    {
        if (last_order) {
            if (*last_order == stm_state::DA)
                return true;
            if (*last_order != stm_state::CCS)
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
    void send_specific_data(json &result);
    void send_message(stm_message *msg);
};
void stm_level_change(level_information newlevel, bool driver);
void stm_level_transition_received(level_transition_information info);
void send_failed_msg(stm_object *stm);
void assign_stm(int nid_ntc, bool driver);
void setup_stm_control();
void update_stm_control();
void handle_stm_message(const stm_message &msg);
void stm_send_train_data();
stm_object *get_stm(int nid_ntc);
std::string get_ntc_name(int nid_ntc);
extern std::map<int, stm_object*> installed_stms;
extern std::map<int, int> ntc_to_stm;
extern std::map<int, std::vector<stm_object*>> ntc_to_stm_lookup_table;
extern bool stm_control_EB;