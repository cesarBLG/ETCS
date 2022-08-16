#pragma once
#include "../Parser/nlohmann/json.hpp"
using json = nlohmann::json;
enum struct dialog_sequence
{
    None,
    StartUp,
    Main,
    Shunting,
    Override,
    Special,
    Settings,
    DataView
};
extern dialog_sequence active_dialog;
extern std::string active_dialog_step;
extern json active_window_dmi;
/*extern json default_window;
extern const json main_window_radio_wait;
extern const json radio_window_radio_wait;*/
/*json main_window();
json menu_radio();
json menu_override();
json driver_id_window(bool trn);
json fixed_train_data_window();
json fixed_train_data_validation_window(std::string data);*/
void update_dmi_windows();
void close_window();
void validate_data_entry(std::string name, json &result);
void update_dialog_step(std::string step, std::string step2);