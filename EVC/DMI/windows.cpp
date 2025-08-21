/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "windows.h"
#include "acks.h"
#include "../language/language.h"
#include "../Supervision/emergency_stop.h"
#include "../Supervision/supervision.h"
#include "../Euroradio/session.h"
#include "../Procedures/start.h"
#include "../Procedures/override.h"
#include "../Procedures/train_trip.h"
#include "../Procedures/level_transition.h"
#include "../Procedures/stored_information.h"
#include "../Packets/vbc.h"
#include "../STM/stm.h"
#include "../TrainSubsystems/train_interface.h"
#include "../Version/version.h"
#include "dmi.h"
#include "platform_runtime.h"
dialog_sequence active_dialog;
std::string active_dialog_step;
json default_window = R"({"active":"default"})"_json;
json active_window_dmi = default_window;
const json main_window_radio_wait = R"({"active":"menu_main","hour_glass":true,"enabled":{"Start":false,"Driver ID":false,"Train Data":false,"Level":false,"Train Running Number":false,"Maintain Shunting":false,"Shunting":false,"Non Leading":false,"Radio Data":false,"Exit":false}})"_json;
const json radio_window_radio_wait = R"({"active":"menu_radio","hour_glass":true,"enabled":{"Exit":false,"Contact last RBC":false,"Use short number":false,"Enter RBC data":false,"Radio Network ID":false}})"_json;
bool pending_train_data_send = false;
bool any_button_pressed_async = false;
bool any_button_pressed = false;
bool flexible_data_entry = false;
bool message_when_driver_id_entered = false;
bool message_when_running_number_entered = false;
bool message_when_train_data_entered = false;
bool message_when_level_selected = false;
int data_entry_type = 0;
int data_entry_type_tiu = -1;
extern bool bot_driver;
std::map<std::string, std::string> const_train_data;
std::map<std::string, std::vector<std::string>> custom_train_data_inputs;

json build_input_field(std::string label, std::string value, std::vector<std::string> values)
{
    json j;
    if (value != "") j["Value"] = value;
    json key;
    key["Type"] = "Dedicated";
    key["Keys"] = values;
    j["Keyboard"] = key;
    j["Label"] = label;
    return j;
}
json build_numeric_field(std::string label, std::string value)
{
    json j;
    if (value != "") j["Value"] = value;
    json key;
    key["Type"] = "Numeric";
    j["Keyboard"] = key;
    j["Label"] = label;
    return j;
}
json build_alphanumeric_field(std::string label, std::string value)
{
    json j;
    if (value != "") j["Value"] = value;
    json key;
    key["Type"] = "Alphanumeric";
    j["Keyboard"] = key;
    j["Label"] = label;
    return j;
}
json build_yesno_field(std::string label, std::string value)
{
    json j;
    if (value != "") j["Value"] = value;
    json key;
    key["Type"] = "YesNo";
    j["Keyboard"] = key;
    j["Label"] = label;
    return j;
}
json build_input_window(std::string name, std::vector<json> inputs)
{
    json def;
    def["WindowType"] = "DataEntry";
    def["WindowTitle"] = name;
    def["Inputs"] = inputs;
    return def;
}
json driver_id_window(bool trn)
{
    json j = R"({"active":"driver_window"})"_json;
    j["show_trn"] = trn;
    j["WindowDefinition"] = build_input_window(get_text("Driver ID"), {build_alphanumeric_field("", driver_id)});
    return j;
}
json level_window()
{
    json j = R"({"active":"level_window"})"_json;
    std::string lv = "";
    switch(level) {
        case Level::N0:
            lv = get_text("Level 0");
            break;
        case Level::N1:
            lv = get_text("Level 1");
            break;
        case Level::N2:
            lv = get_text("Level 2");
            break;
        case Level::N3:
            lv = get_text("Level 3");
            break;
        case Level::NTC: {
            lv = get_ntc_name(nid_ntc);
            break;
        }
    }
    std::vector<std::string> levels(4);
    if (priority_levels_valid) {
        for (auto lti : priority_levels) {
            switch(lti.level) {
                case Level::N0:
                    levels[3] = get_text("Level 0");
                    break;
                case Level::N1:
                    levels[0] = get_text("Level 1");
                    break;
                case Level::N2:
                    levels[1] = get_text("Level 2");
                    break;
                case Level::N3:
                    levels[2] = get_text("Level 3");
                    break;
                case Level::NTC: {
                    std::vector<int> stms;
                    if (ntc_to_stm_lookup_table.find(lti.nid_ntc) != ntc_to_stm_lookup_table.end())
                        stms = ntc_to_stm_lookup_table[lti.nid_ntc];
                    stms.push_back(lti.nid_ntc);
                    for (int nid : stms) {
                        if (installed_stms.find(nid) != installed_stms.end() || ntc_available_no_stm.find(lti.nid_ntc) != ntc_available_no_stm.end()) {
                            levels.push_back(get_ntc_name(lti.nid_ntc));
                            break;
                        }
                    }
                    break;
                }
            }
        }
    } else {
        if (!unsupported_levels.count(0))
            levels[3] = get_text("Level 0");
        if (!unsupported_levels.count(1))
            levels[0] = get_text("Level 1");
        if (!unsupported_levels.count(2))
            levels[1] = get_text("Level 2");
        if (!unsupported_levels.count(3) && false)
            levels[2] = get_text("Level 3");
        for (auto &kvp : installed_stms) {
            levels.push_back(get_ntc_name(kvp.first));
        }
        for (int ntc : ntc_available_no_stm) {
            levels.push_back(get_ntc_name(ntc));
        }
    }
    j["WindowDefinition"] = build_input_window(get_text("Level"), {build_input_field("", lv, levels)});
    return j;
}
json trn_window()
{
    json j = R"({"active":"trn_window"})"_json;
    j["WindowDefinition"] = build_input_window(get_text("Train running number"), {build_numeric_field("", train_running_number_valid ? std::to_string(train_running_number) : "")});
    return j;
}
json rbc_data_window()
{
    json j = R"({"active":"rbc_data_window"})"_json;
    std::vector<json> inputs = {
        build_numeric_field(get_text("RBC ID"), rbc_contact ? std::to_string(rbc_contact->country<<14 | rbc_contact->id) : ""),
        build_numeric_field(get_text("RBC phone number"), rbc_contact ? from_bcd(rbc_contact->phone_number) : "")
    };
    inputs[0]["Echo"] = false;
    inputs[1]["Echo"] = false;
    j["WindowDefinition"] = build_input_window(get_text("RBC data"), inputs);
    return j;
}
json radio_network_window()
{
    json j = R"({"active":"radio_network_window"})"_json;
    std::vector<std::string> ids;
    for (auto &id : *AllowedRadioNetworks) {
        ids.push_back(radio_network_name(id));
    }
    j["WindowDefinition"] = build_input_window(get_text("Radio network ID"), {build_input_field("", radio_network_name(RadioNetworkId), ids)});
    return j;
}
json fixed_train_data_window()
{
    json j = R"({"active":"fixed_train_data_window"})"_json;
    std::vector<json> inputs;

    json j2;
    auto contents = platform->read_file(traindata_file);
    if (contents)
        j2 = json::parse(*contents);
    std::vector<std::string> types;
    for (auto it = j2.begin(); it!=j2.end(); ++it) {
        types.push_back(it.key());
    }
    j["WindowDefinition"] = build_input_window(get_text("Train data"), {build_input_field(get_text("Train type"), special_train_data, types)});
    j["Switchable"] = data_entry_type == 2;
    return j;
}
json train_data_window()
{
    json j = R"({"active":"train_data_window"})"_json;
    std::vector<json> inputs;
    
    if (!const_train_data.count("TrainCategory")) {
        std::vector<std::string> categories = {
            get_text("PASS 1"), get_text("PASS 2"), get_text("PASS 3"),
            get_text("TILT 1"), get_text("TILT 2"), get_text("TILT 3"),
            get_text("TILT 4"), get_text("TILT 5"), get_text("TILT 6"),
            get_text("TILT 7"), get_text("FP 1"), get_text("FP2"),
            get_text("FP 3"), get_text("FP 4"), get_text("FG 1"),
            get_text("FG 2"), get_text("FG 3"), get_text("FG 4")
        };
        if (custom_train_data_inputs.count("TrainCategory")) {
            categories.clear();
            for (auto cat : custom_train_data_inputs["TrainCategory"]) {
                categories.push_back(get_text(cat));
            }
        }
        inputs.push_back(build_input_field(get_text("Train category"), train_data_known ? train_category : "", categories));
    }

    inputs.push_back(build_numeric_field(get_text("Length (m)"), train_data_known ? std::to_string((int)L_TRAIN) : ""));
    inputs.push_back(build_numeric_field(get_text("Brake percentage"), train_data_known ? std::to_string(brake_percentage) : ""));
    inputs.push_back(build_numeric_field(get_text("Max speed (km/h)"), train_data_known ? std::to_string((int)(V_train*3.6)) : ""));

    if (!const_train_data.count("AxleLoadCategory")) {
        std::vector<std::string> categories = { get_text("A"), get_text("HS17"), get_text("B1"), get_text("B2"), get_text("C2"), get_text("C3"), get_text("C4"), get_text("D2"), get_text("D3"), get_text("D4"), get_text("D4XL"), get_text("E4"), get_text("E5") };
        std::string cat = train_data_known ? categories[(int)axle_load_category] : "";
        if (custom_train_data_inputs.count("AxleLoadCategory")) {
            categories.clear();
            for (auto cat : custom_train_data_inputs["AxleLoadCategory"]) {
                categories.push_back(get_text(cat));
            }
        }
        inputs.push_back(build_input_field(get_text("Axle load category"), cat, categories));
    }

    if (!const_train_data.count("Airtight")) {
        std::string air = train_data_known ? (Q_airtight ? get_text("Yes") : get_text("No")) : "";
        inputs.push_back(build_yesno_field(get_text("Airtight"), air));
    }

    if (!const_train_data.count("LoadingGauge")) {
        std::vector<std::string> gauges = { get_text("G1"), get_text("GA"), get_text("GB"), get_text("GC"), get_text("Out of GC") };
        std::string gauge = train_data_known ? gauges[(int)loading_gauge] : "";
        if (custom_train_data_inputs.count("LoadingGauge")) {
            gauges.clear();
            for (auto g : custom_train_data_inputs["LoadingGauge"]) {
                gauges.push_back(get_text(g));
            }
        }
        inputs.push_back(build_input_field(get_text("Loading gauge"), gauge, gauges));
    }

    j["WindowDefinition"] = build_input_window(get_text("Train data"), inputs);
    j["Switchable"] = data_entry_type == 2;
    return j;
}
json ntc_menu(bool hourglass)
{
    json j = R"({"active":"menu_ntc"})"_json;
    j["hour_glass"] = hourglass;
    for (auto &kvp : installed_stms) {
        j["STMs"].push_back(get_ntc_name(kvp.first));
    }
    return j;
}
json ntc_data_window()
{
    for (auto &kvp : installed_stms) {
        auto *stm = kvp.second;
        if (stm->data_entry == stm_object::data_entry_state::Driver) {
            json j = R"({"active":"ntc_data_window"})"_json;
            j["ntc"] = get_ntc_name(kvp.first);
            std::vector<json> inputs;
            for (auto &field : stm->specific_data) {
                if (field.keys.empty())
                    inputs.push_back(build_alphanumeric_field(field.caption, field.value));
                else
                    inputs.push_back(build_input_field(field.caption, field.value, field.keys));
            }
            j["WindowDefinition"] = build_input_window(get_ntc_name(kvp.first)+get_text(" data"), inputs);
            return j;
        }
    }
    return json();
}
json adhesion_window()
{
    json j = R"({"active":"adhesion_window"})"_json;
    j["WindowDefinition"] = build_input_window(get_text("Adhesion"), {build_input_field("", slippery_rail_driver ? get_text("Slippery rail") : get_text("Non slippery rail"), {get_text("Non slippery rail"), get_text("Slippery rail")})});
    return j;
}
json sr_data_window()
{
    json j = R"({"active":"sr_data_window"})"_json;
    std::vector<json> inputs = {
        build_numeric_field(get_text("SR speed (km/h)"), SR_speed ? std::to_string((int)(SR_speed->speed*3.6)) : ""),
        build_numeric_field(get_text("SR distance (m)"), std::to_string(SR_dist ? std::min(std::max((int)(SR_dist->est-d_estfront), 0), 100000) : 100000))
    };
    j["WindowDefinition"] = build_input_window(get_text("SR speed/distance"), inputs);
    return j;
}
json language_window()
{
    json j = R"({"active":"language_window"})"_json;
    j["WindowDefinition"] = build_input_window(get_text("Language"), {build_input_field("", language, {"en","es","fr","de","pt","it","nl","sv","el","hu","pl"})});
    return j;
}
json set_vbc_window()
{
    json j = R"({"active":"set_vbc_window"})"_json;
    j["WindowDefinition"] = build_input_window(get_text("Set VBC"), {build_numeric_field(get_text("VBC code"), "")});
    return j;
}
json remove_vbc_window()
{
    json j = R"({"active":"remove_vbc_window"})"_json;
    j["WindowDefinition"] = build_input_window(get_text("Remove VBC"), {build_numeric_field(get_text("VBC code"), "")});
    return j;
}
json build_field(std::string label, std::string value)
{
    json j;
    j["Label"] = label;
    j["Value"] = value;
    return j;
}
json build_data_view_window(std::string name, std::vector<json> fields)
{
    json def;
    def["WindowType"] = "DataView";
    def["WindowTitle"] = name;
    def["Fields"] = fields;
    return def;
}
json data_view_window()
{
    json j = R"({"active":"data_view_window"})"_json;
    std::vector<json> fields;
    {
        fields.push_back(build_alphanumeric_field(get_text("Driver ID"), driver_id));
        fields.push_back(build_field("", ""));
        fields.push_back(build_numeric_field(get_text("Train running number"), std::to_string(train_running_number)));
        if (train_data_known) {
            fields.push_back(build_field("", ""));
            if (special_train_data != "") fields.push_back(build_field(get_text("Train type"), special_train_data));
            if (train_category != "" && !const_train_data.count("TrainCategory")) fields.push_back(build_field(get_text("Train category"), get_text(train_category)));
            if (L_TRAIN > 0 && !const_train_data.count("Length")) fields.push_back(build_numeric_field(get_text("Length (m)"), std::to_string((int)L_TRAIN)));
            if (brake_percentage > 0 && !const_train_data.count("BrakePercentage")) fields.push_back(build_numeric_field(get_text("Brake percentage"), std::to_string(brake_percentage)));
            if (V_train > 0 && !const_train_data.count("MaxSpeed")) fields.push_back(build_numeric_field(get_text("Maximum speed (km/h)"), std::to_string((int)(V_train*3.6))));
            if (!const_train_data.count("AxleLoadCategory")) {
                std::vector<std::string> categories = { get_text("A"), get_text("HS17"), get_text("B1"), get_text("B2"), get_text("C2"), get_text("C3"), get_text("C4"), get_text("D2"), get_text("D3"), get_text("D4"), get_text("D4XL"), get_text("E4"), get_text("E5") };
                fields.push_back(build_field(get_text("Axle load category"), categories[(int)axle_load_category]));
            }
            if (!const_train_data.count("Airtight")) fields.push_back(build_field(get_text("Airtight"), Q_airtight ? get_text("Yes") : get_text("No")));
            if (!const_train_data.count("LoadingGauge")) {
                std::vector<std::string> gauges = { get_text("G1"), get_text("GA"), get_text("GB"), get_text("GC"), get_text("Out of GC") };
                fields.push_back(build_field(get_text("Loading gauge"), gauges[(int)loading_gauge]));
            }
        }
        for (int i=fields.size(); i<14; i++) {
            fields.push_back(build_field("", ""));
        }
        fields.push_back(build_field(get_text("Radio network ID"), radio_network_name(RadioNetworkId)));
        if (rbc_contact) {
            fields.push_back(build_numeric_field(get_text("RBC ID"), std::to_string(rbc_contact->id)));
            fields.push_back(build_numeric_field(get_text("RBC phone number"), from_bcd(rbc_contact->phone_number)));
            
        }
        fields.push_back(build_field("", ""));
        int i = 1;
        for (auto &vbc : vbcs) {
            fields.push_back(build_numeric_field("VBC #"+std::to_string(i++)+" set code", std::to_string(vbc.NID_VBCMK|(vbc.NID_C<<6)|((vbc.validity/86400000LL)<<16))));
        }
    }
    j["WindowDefinition"] = build_data_view_window(get_text("Data view"), fields);
    return j;
}
json system_version_window()
{
    json j = R"({"active":"system_version_window"})"_json;
    j["WindowDefinition"] = build_data_view_window(get_text("System version"), {build_field(get_text("Operated system version"), std::to_string(VERSION_X(operated_version))+"."+std::to_string(VERSION_Y(operated_version)))});
    return j;
}
static dialog_sequence prev_dialog;
static std::string prev_step;
void update_dmi_windows()
{
    update_acks();
    any_button_pressed = any_button_pressed_async;
    any_button_pressed_async = false;
    //json prev_active = active_window_dmi;
    bool changed = prev_dialog != active_dialog || prev_step != active_dialog_step;
    prev_dialog = active_dialog;
    prev_step = active_dialog_step;
    if (active_dialog == dialog_sequence::None) {
        if (changed) active_window_dmi = default_window;
    } else if (active_dialog == dialog_sequence::StartUp) {
        if (!som_active)
            active_dialog = dialog_sequence::None;
        if (active_dialog_step == "S0") {
            if (changed)
                active_window_dmi = main_window_radio_wait;
            if (som_status != S0)
                active_dialog_step = "S1";
        } else if (active_dialog_step == "S1") {
            if (changed)
                active_window_dmi = driver_id_window(true);
            if (som_status != S1)
                active_dialog_step = "D2";
        } else if (active_dialog_step == "S1-1") {
            active_dialog = dialog_sequence::Settings;
            active_dialog_step = "S1";
        } else if (active_dialog_step == "S1-2") {
            if (changed)
                active_window_dmi = trn_window();
        } else if (active_dialog_step == "S2") {
            if (changed)
                active_window_dmi = level_window();
            if (som_status != S2) {
                if (level == Level::N2 || level == Level::N3)
                    active_dialog_step = "S3-1";
                else
                    active_dialog_step = "S10";
            }
        } else if (active_dialog_step == "S3-1") {
            if (changed)
                active_window_dmi = R"({"active":"menu_radio"})"_json;
            if (som_status != S3)
                active_dialog_step = "A31";
        } else if (active_dialog_step == "S3-2-1") {
            if (changed)
                active_window_dmi = radio_window_radio_wait;
            if (AllowedRadioNetworks) {
                if (AllowedRadioNetworks->empty()) {
                    som_status = A29;
                    active_dialog_step = "A29";
                } else {
                    active_dialog_step = "S3-2-2";
                }
            }
        } else if (active_dialog_step == "S3-2-2") {
            if (changed)
                active_window_dmi = radio_network_window();
        } else if (active_dialog_step == "S3-2-3") {
            if (changed)
                active_window_dmi = radio_window_radio_wait;
            bool registered = false;
            bool timeout = true;
            for (mobile_terminal *t : mobile_terminals) {
                if (t->registered) {
                    registered = true;
                    break;
                }
                if (t->last_register_order && get_milliseconds() - *t->last_register_order < T_network_registration * 1000)
                    timeout = false;
            }
            if (registered || timeout)
                active_dialog_step = "S3-1";
        } else if (active_dialog_step == "S3-3") {
            if (changed)
                active_window_dmi = rbc_data_window();
            if (som_status != S3)
                active_dialog_step = "A31";
        } else if (active_dialog_step == "S4") {
            if (changed)
                active_window_dmi = main_window_radio_wait;
            if (som_status == A29)
                active_dialog_step = "A29";
            else if (som_status == A31)
                active_dialog_step = "A31";
        } else if (active_dialog_step == "A29") {
            add_message(text_message(get_text("Radio network registration failed"), true, false, 0, [](text_message &t){return any_button_pressed;}));
            RadioNetworkId = 0;
            active_dialog_step = "S10";
        } else if (active_dialog_step == "A31") {
            if (changed)
                active_window_dmi = main_window_radio_wait;
            if (som_status != A31)
                active_dialog_step = "D31";
        } else if (active_dialog_step == "A32") {
            active_dialog_step = "S10";
        } else if (active_dialog_step == "A40") {
            add_message(text_message(get_text("Train is rejected"), true, false, 0, [](text_message &t){return any_button_pressed;})); // TODO
            active_dialog_step = "S10";
        } else if (active_dialog_step == "D2") {
            if (som_status == S2)
                active_dialog_step = "S2";
            else if (som_status != D2)
                active_dialog_step = "D3";
        } else if (active_dialog_step == "D3") {
            if (level == Level::N2 || level == Level::N3)
                active_dialog_step = "D7";
            else
                active_dialog_step = "S10";
        } else if (active_dialog_step == "D7") {
            bool registered = false;
            for (mobile_terminal *t : mobile_terminals) {
                if (t->registered) {
                    registered = true;
                    break;
                }
            }
            if (registered)
                active_dialog_step = "A31";
            else
                active_dialog_step = "S4";
        } else if (active_dialog_step == "D31") {
            if (supervising_rbc && supervising_rbc->status == session_status::Established)
                active_dialog_step = "D32";
            else
                active_dialog_step = "A32";
        } else if (active_dialog_step == "D32") {
            if (som_status == A40)
                active_dialog_step = "A40";
            if (som_status == S10)
                active_dialog_step = "S10";
        } else if (active_dialog_step == "S10") {
            active_dialog = dialog_sequence::Main;
            active_dialog_step = "S1";
        }
        active_window_dmi["enabled"]["Exit"] = active_dialog_step == "S1-1" || active_dialog_step == "S1-2" || active_dialog_step == "S3-2-2" || active_dialog_step == "S3-3";
    } else if (active_dialog == dialog_sequence::Main) {
        if (active_dialog_step == "S1") {
            if (changed)
                active_window_dmi = R"({"active":"menu_main"})"_json;
        } else if (active_dialog_step == "S2") {
            if (changed)
                active_window_dmi = driver_id_window(false);
        } else if (active_dialog_step == "S3-1") {
            if (changed) {
                if (data_entry_type == 0)
                    flexible_data_entry = true;
                else if (data_entry_type == 1)
                    flexible_data_entry = false;
                if (flexible_data_entry)
                    active_window_dmi = train_data_window();
                else
                    active_window_dmi = fixed_train_data_window();
            }
        } else if (active_dialog_step == "S3-2") {
            //active_window_dmi = fixed_train_data_validation_window();
        } else if (active_dialog_step == "S3-3") {
            if (changed)
                active_window_dmi = trn_window();
        } else if (active_dialog_step == "S4") {
            if (changed)
                active_window_dmi = level_window();
        } else if (active_dialog_step == "S5-1") {
            if (changed)
                active_window_dmi = R"({"active":"menu_radio"})"_json;
        } else if (active_dialog_step == "S5-2-1") {
            if (changed)
                active_window_dmi = radio_window_radio_wait;
            if (AllowedRadioNetworks) {
                if (AllowedRadioNetworks->empty()) {
                    active_dialog_step = "A5";
                } else {
                    active_dialog_step = "S5-2-2";
                }
            }
        } else if (active_dialog_step == "S5-2-2") {
            if (changed)
                active_window_dmi = radio_network_window();
        } else if (active_dialog_step == "S5-2-3") {
            if (changed)
                active_window_dmi = radio_window_radio_wait;
            bool registered = false;
            bool timeout = true;
            for (mobile_terminal *t : mobile_terminals) {
                if (t->registered) {
                    registered = true;
                    break;
                }
                if (t->last_register_order && get_milliseconds() - *t->last_register_order < T_network_registration * 1000)
                    timeout = false;
            }
            if (registered || timeout)
                active_dialog_step = "S5-1";
        } else if (active_dialog_step == "A5") {
            add_message(text_message(get_text("Radio network registration failed"), true, false, 0, [](text_message &t){return any_button_pressed;}));
            RadioNetworkId = 0;
            active_dialog_step = "S1";
        } else if (active_dialog_step == "S5-3") {
            if (changed)
                active_window_dmi = rbc_data_window();
        } else if (active_dialog_step == "S6") {
            if (changed)
                active_window_dmi = trn_window();
        } else if (active_dialog_step == "S7") {
            if (changed)
                active_window_dmi = main_window_radio_wait;
            if (!supervising_rbc || supervising_rbc->status == session_status::Inactive)
                active_dialog_step = "S1";
        } else if (active_dialog_step == "S8") {
            if (changed)
                active_window_dmi = main_window_radio_wait;
            if (!supervising_rbc || supervising_rbc->status != session_status::Establishing)
                active_dialog_step = "D3";
        } else if (active_dialog_step == "S9") {
            if (changed)
                active_window_dmi = main_window_radio_wait;
            if (!supervising_rbc || supervising_rbc->status == session_status::Inactive || !supervising_rbc->train_data_ack_pending)
                active_dialog_step = "S1";
        } else if (active_dialog_step == "D1") {
            if (supervising_rbc) {
                supervising_rbc->train_data_ack_pending = true;
                supervising_rbc->train_data_ack_sent = false;
                supervising_rbc->train_running_number_sent = false;
            }
            if (level == Level::N2 || level == Level::N3)
                active_dialog_step = "D2";
            else
                active_dialog_step = "S1";
            if (train_data_valid && som_active) {
                for (auto kvp : installed_stms) {
                    auto *stm = kvp.second;
                    if (!stm->isolated && stm->state != stm_state::CO && stm->state != stm_state::CS && stm->state != stm_state::HS && stm->state != stm_state::DA)
                        send_failed_msg(stm);
                }
            }
        } else if (active_dialog_step == "D2") {
            if (supervising_rbc && supervising_rbc->status == session_status::Established)
                active_dialog_step = "S9";
            else
                active_dialog_step = "S1";
        } else if (active_dialog_step == "D3") {
            if (supervising_rbc && supervising_rbc->status == session_status::Established)
                active_dialog_step = "D4";
            else if (!supervising_rbc || supervising_rbc->status == session_status::Inactive)
                active_dialog_step = "S1";
        } else if (active_dialog_step == "D4") {
            if (som_active) {
                active_dialog = dialog_sequence::StartUp;
                active_dialog_step = "D32";
                som_status = D32;
            } else {
                active_dialog_step = "S1";
            }
        } else if (active_dialog_step == "D5") {
            bool registered = false;
            for (mobile_terminal *t : mobile_terminals) {
                if (t->registered) {
                    registered = true;
                    break;
                }
            }
            if (rbc_contact && rbc_contact_valid && registered) {
                set_supervising_rbc(*rbc_contact);
                supervising_rbc->open(N_tries_radio);
                active_dialog_step = "S8";
            } else {
                active_dialog_step = "S5-1";
            }
        } else if (active_dialog_step == "D6") {
            if (train_running_number_valid)
                active_dialog_step = "D1";
            else
                active_dialog_step = "S3-3";
        } else if (active_dialog_step == "D7") {
            if (supervising_rbc && supervising_rbc->status == session_status::Established)
                active_dialog_step = "S7";
            else
                active_dialog = dialog_sequence::None;
        }
        active_window_dmi["enabled"]["Exit"] = active_dialog_step != "S5-2-1" && active_dialog_step != "S5-2-3" && active_dialog_step != "S7" && active_dialog_step != "S8" && active_dialog_step != "S9";
    } else if (active_dialog == dialog_sequence::NTCData) {
        if (active_dialog_step == "S1") {
            if (changed)
                active_window_dmi = ntc_menu(true);
            bool waiting=false;
            for (auto &kvp : installed_stms) {
                auto *stm = kvp.second;
                if (stm->data_entry == stm_object::data_entry_state::Start) {
                    waiting = true;
                    break;
                }
            }
            if (!waiting)
             active_dialog_step = "S2";
        } else if (active_dialog_step == "S2") {
            if (changed)
                active_window_dmi = ntc_menu(false);
            bool remaining = false;
            for (auto kvp : installed_stms) {
                auto *stm = kvp.second;
                if (stm->data_entry != stm_object::data_entry_state::Inactive) {
                    remaining = true;
                    break;
                }
            }
            if (!remaining) {
                active_dialog = dialog_sequence::Main;
                active_dialog_step = "D6";
            }
        } else if (active_dialog_step == "D1") {
            bool needsdata = false;
            for (auto &kvp : installed_stms) {
                auto *stm = kvp.second;
                if (stm->specific_data_need > 0 && (stm->state == stm_state::CO || stm->state == stm_state::DE || stm->state == stm_state::CS || stm->state == stm_state::HS || stm->state == stm_state::DA)) {
                    needsdata = true;
                    break;
                }
            }
            if (needsdata) {
                active_dialog_step = "S1";
            } else {
                active_dialog = dialog_sequence::Main;
                active_dialog_step = "D6";
            }
        } else if (active_dialog_step == "S3-1") {
            if (changed)
                active_window_dmi = ntc_data_window();
        } else if (active_dialog_step == "S4") {
            if (changed)
                active_window_dmi = ntc_menu(true);
            bool wait = false;
            for (auto kvp : installed_stms) {
                auto *stm = kvp.second;
                if (stm->data_entry == stm_object::data_entry_state::DataSent) {
                    wait = true;
                    break;
                }
            }
            if (!wait)
                active_dialog_step = "S2";
        }
        active_window_dmi["enabled"]["Exit"] = active_dialog_step != "S1" && active_dialog_step != "S4";
    } else if (active_dialog == dialog_sequence::Override) {
        if (active_dialog_step == "S1")
            if (changed)
                active_window_dmi = R"({"active":"menu_override"})"_json;
    } else if (active_dialog == dialog_sequence::Shunting) {
        if (active_dialog_step == "D1") {
            if (level == Level::N2 || level == Level::N3) {
                active_dialog_step = "S1";
            } else/* if (level == level::N0 || level == level::N1) */{
                active_dialog = dialog_sequence::None;
            }
        } else if (active_dialog_step == "S1") {
            if (changed)
                active_window_dmi = main_window_radio_wait;
            if (!supervising_rbc || supervising_rbc->status != session_status::Established) {
                active_dialog = dialog_sequence::Main;
                active_dialog_step = "S1";
                add_message(text_message(get_text("Shunting request failed"), true, false, 0, [](text_message &t){return any_button_pressed;}));
            }
        }
    } else if (active_dialog == dialog_sequence::DataView) {
        if (changed)
                active_window_dmi = data_view_window();
    } else if (active_dialog == dialog_sequence::Special) {
        if (active_dialog_step == "S1") {
            if (changed)
                active_window_dmi = R"({"active":"menu_spec"})"_json;
        } else if (active_dialog_step == "S2") {
            if (changed)
                active_window_dmi = adhesion_window();
        } else if (active_dialog_step == "S3") {
            if (changed)
                active_window_dmi = sr_data_window();
        }
    } else if (active_dialog == dialog_sequence::Settings) {
        if (active_dialog_step == "S1") {
            if (changed)
                active_window_dmi = R"({"active":"menu_settings"})"_json;
        } else if (active_dialog_step == "S2") {
            if (changed)
                active_window_dmi = language_window();
        } else if (active_dialog_step == "S3") {
            if (changed)
                active_window_dmi = R"({"active":"volume_window"})"_json;
        } else if (active_dialog_step == "S4") {
            if (changed)
                active_window_dmi = R"({"active":"brightness_window"})"_json;
        } else if (active_dialog_step == "S5") {
            if (changed)
                active_window_dmi = system_version_window();
        } else if (active_dialog_step == "S6-1") {
            if (changed)
                active_window_dmi = set_vbc_window();
        } else if (active_dialog_step == "S6-2") {

        } else if (active_dialog_step == "S7-1") {
            if (changed)
                active_window_dmi = remove_vbc_window();
        } else if (active_dialog_step == "S7-2") {

        }
    }
    std::map<std::string, bool> enabled_buttons;
    if (active_dialog != dialog_sequence::None) {
        bool c1 = V_est == 0 && mode == Mode::SB && train_data_valid && level != Level::Unknown;
        bool c2 = V_est == 0 && mode == Mode::PT && train_data_valid && (level == Level::N1 || ((level == Level::N2 || level == Level::N3) && trip_exit_acknowledged && supervising_rbc && supervising_rbc->status == session_status::Established && emergency_stops.empty()));
        bool c3 = mode == Mode::SR && (level == Level::N2 || level == Level::N3) && supervising_rbc && supervising_rbc->status == session_status::Established;
        enabled_buttons["Start"] = c1 || c2 || c3;
        enabled_buttons["Driver ID"] = (V_est == 0 && mode == Mode::SB && driver_id_valid && level_valid) || ((M_NVDERUN || (!M_NVDERUN && V_est == 0)) &&
            (mode == Mode::SH || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::UN || mode == Mode::SN));
        enabled_buttons["Train Data"] = V_est == 0 && driver_id_valid && level_valid &&
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::UN || mode == Mode::SN);
        enabled_buttons["Maintain Shunting"] = mode == Mode::SH && false;
        enabled_buttons["Level"] = V_est == 0 && driver_id_valid &&
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::UN || mode == Mode::SN);
        enabled_buttons["Train Running Number"] = (V_est == 0 && mode == Mode::SB && driver_id_valid && level_valid) ||
            (mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::UN || mode == Mode::SN);
        enabled_buttons["Shunting"] = (mode == Mode::SH && V_est == 0) ||
            (V_est == 0 && driver_id_valid && (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::UN || mode == Mode::SN)
                && level_valid && (level == Level::N0 || level == Level::N1 || level == Level::NTC || ((level == Level::N2 || level == Level::N3) && supervising_rbc && supervising_rbc->status == session_status::Established))) ||
            (V_est == 0 && mode == Mode::PT && (level == Level::N1 || ((level == Level::N2 || level == Level::N3) && trip_exit_acknowledged && supervising_rbc && supervising_rbc->status == session_status::Established && emergency_stops.empty())));
        enabled_buttons["Non Leading"] = V_est == 0 && driver_id_valid && level_valid && (mode == Mode::SB || mode == Mode::SH || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS) && nl_signal;
        enabled_buttons["Radio Data"] = V_est == 0 && driver_id_valid && level_valid &&
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::PT || mode == Mode::UN || mode == Mode::SN);
        
        bool registered = false;
        for (mobile_terminal *t : mobile_terminals) {
            if (t->registered) {
                registered = true;
                break;
            }
        }
        enabled_buttons["Contact last RBC"] = V_est == 0 && driver_id_valid && 
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::PT) && 
            level_valid && (level == Level::N2 || level == Level::N3) && registered && rbc_contact;
        enabled_buttons["Use short number"] = V_est == 0 && driver_id_valid && 
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::PT) && 
            level_valid && (level == Level::N2 || level == Level::N3) && registered;
        enabled_buttons["Enter RBC data"] = V_est == 0 && driver_id_valid && 
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::PT) && 
            level_valid && (level == Level::N2 || level == Level::N3) && registered;
        enabled_buttons["Radio Network ID"] = V_est == 0 && driver_id_valid && 
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::PT) && 
            level_valid;

        enabled_buttons["Adhesion"] = (V_est == 0 && mode == Mode::SB && Q_NVDRIVER_ADHES && driver_id_valid && train_data_valid && level_valid) || (Q_NVDRIVER_ADHES && (mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::UN || mode == Mode::SN));
        enabled_buttons["SRspeed"] = V_est == 0 && mode == Mode::SR;
        enabled_buttons["TrainIntegrity"] = V_est == 0 && (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::PT) && driver_id_valid && train_data_valid && level_valid;

        bool c = (V_est == 0 && mode == Mode::SB) || (mode == Mode::SH || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::UN || mode == Mode::TR || mode == Mode::PT || mode == Mode::SN || mode == Mode::RV);
        enabled_buttons["Language"] = c;
        enabled_buttons["Volume"] = c;
        enabled_buttons["Brightness"] = c;
        enabled_buttons["SystemVersion"] = c;
        enabled_buttons["SetVBC"] = V_est == 0 && mode == Mode::SB;
        enabled_buttons["RemoveVBC"] = V_est == 0 && mode == Mode::SB && !vbcs.empty();


        for (auto &kvp : installed_stms) {
            auto *stm = kvp.second;
            enabled_buttons[get_ntc_name(kvp.first)] = stm->data_entry == stm_object::data_entry_state::Active || stm->data_entry == stm_object::data_entry_state::Driver;
        }
        enabled_buttons["EndDataEntry"] = active_dialog_step != "S1" && active_dialog_step != "S4";
    }
    if (ack_required && active_dialog != dialog_sequence::StartUp) {
        for (auto &kvp : enabled_buttons) {
            kvp.second = false;
        }
    }
    std::string active = active_window_dmi["active"];
    if (active == "menu_main" && !active_window_dmi.contains("hour_glass")) {
        json &enabled = active_window_dmi["enabled"];
        enabled["Start"] = enabled_buttons["Start"];
        enabled["Driver ID"] = enabled_buttons["Driver ID"];
        enabled["Train Data"] = enabled_buttons["Train Data"];
        enabled["Maintain Shunting"] = enabled_buttons["Maintain Shunting"];
        enabled["Level"] = enabled_buttons["Level"];
        enabled["Train Running Number"] = enabled_buttons["Train Running Number"];
        enabled["Shunting"] = enabled_buttons["Shunting"];
        enabled["Non Leading"] = enabled_buttons["Non Leading"];
        enabled["Radio Data"] = enabled_buttons["Radio Data"];
    } else if (active == "menu_radio" && !active_window_dmi.contains("hour_glass")) {
        json &enabled = active_window_dmi["enabled"];
        enabled["Contact last RBC"] = enabled_buttons["Contact last RBC"];
        enabled["Use short number"] = enabled_buttons["Use short number"];
        enabled["Enter RBC data"] = enabled_buttons["Enter RBC data"];
        enabled["Radio Network ID"] = enabled_buttons["Radio Network ID"];
    } else if (active == "menu_override") {
        active_window_dmi["enabled"]["EoA"] = V_est <= V_NVALLOWOVTRP && (((mode == Mode::FS || mode == Mode::OS || mode == Mode::LS || mode == Mode::SR || mode == Mode::UN || mode == Mode::PT || mode == Mode::SB || mode == Mode::SN) && train_data_valid) || mode == Mode::SH);
    } else if (active == "menu_spec") {
        json &enabled = active_window_dmi["enabled"];
        enabled["Adhesion"] = enabled_buttons["Adhesion"];
        enabled["SRspeed"] = enabled_buttons["SRspeed"];
        enabled["TrainIntegrity"] = enabled_buttons["TrainIntegrity"];
    } else if (active == "menu_settings") {
        json &enabled = active_window_dmi["enabled"];
        enabled["Language"] = enabled_buttons["Language"];
        enabled["Volume"] = enabled_buttons["Volume"];
        enabled["Brightness"] = enabled_buttons["Brightness"];
        enabled["SystemVersion"] = enabled_buttons["SystemVersion"];
        enabled["SetVBC"] = enabled_buttons["SetVBC"];
        enabled["RemoveVBC"] = enabled_buttons["RemoveVBC"];
    } else if (active == "menu_ntc") {
        json &enabled = active_window_dmi["enabled"];
        for (auto &kvp : installed_stms) {
            auto *stm = kvp.second;
            enabled[get_ntc_name(kvp.first)] = enabled_buttons[get_ntc_name(kvp.first)];
        }
        enabled["EndDataEntry"] = enabled_buttons["EndDataEntry"];
    }
    if (active_dialog != dialog_sequence::None && active_dialog != dialog_sequence::StartUp && (!som_active || som_status != S1)) {
        extern bool traindata_applied;
        bool close = false;
        if ((active == "trn_window" && !enabled_buttons["Train Running Number"])
        || (active == "driver_window" && !enabled_buttons["Driver ID"])
        || (active == "level_window" && !enabled_buttons["Level"])
        || ((active == "train_data_validation_window" || active == "train_data_window" || active == "fixed_train_data_validation_window" || active == "fixed_train_data_window") && !enabled_buttons["Train Data"])) {
            close = true;
            if ((active == "fixed_train_data_window" || active == "fixed_train_data_validation_window" || active == "train_data_window" || active == "train_data_validation_window") && V_est != 0) {
                traindata_applied = true;
                trigger_brake_reason(1);
            }
        }
        if ((active == "rbc_data_window" && !enabled_buttons["Enter RBC data"])
        || (active == "radio_network_window" && !enabled_buttons["Radio Network ID"]) ) {
            close = true;
        }
        if ((active == "language_window" && !enabled_buttons["Language"])
        || (active == "volume_window" && !enabled_buttons["Volume"])
        || (active == "brightness_window" && !enabled_buttons["Brightness"]) ) {
            close = true;
        }
        if ((active == "adhesion_window" && !enabled_buttons["Adhesion"])
        || (active == "sr_data_window" && !enabled_buttons["SRspeed"]) ) {
            close = true;
            if (active == "sr_data_window" && V_est != 0) {
                traindata_applied = true;
                trigger_brake_reason(1);
            }
        }
        if ((active == "ntc_data_window" || active == "ntc_data_validation_window") && !enabled_buttons[active_window_dmi["ntc"]]) {
            close = true;
            if (V_est != 0) {
                traindata_applied = true;
                trigger_brake_reason(1);
            }
        }
        if (close)
            close_window();
    }
}
void close_window()
{
    std::string active = active_window_dmi["active"];
    if (active == "menu_main" || active == "menu_override" || active == "data_view_window" || active == "menu_spec")
        active_dialog = dialog_sequence::None;
    else if (active == "trn_window" || active == "driver_window") {
        active_dialog_step = "S1";   
    } else if (active == "fixed_train_data_window" || active == "level_window" || active == "fixed_train_data_validation_window" || active == "train_data_window" || active == "train_data_validation_window")
        active_dialog_step = "S1";
    else if (active == "menu_ntc") {
        active_dialog = dialog_sequence::Main;
        active_dialog_step = "S1";
    } else if (active == "ntc_data_window" || active == "ntc_data_validation_window") {
        active_dialog_step = "S2";
        for (auto &kvp : installed_stms) {
            auto *stm = kvp.second;
            if (stm->data_entry == stm_object::data_entry_state::Driver)
                stm->data_entry = stm_object::data_entry_state::Active;
        }
    } else if (active == "menu_radio")
        active_dialog_step = "S1";
    else if (active == "rbc_data_window" || active == "radio_network_window") {
        if (active_dialog == dialog_sequence::StartUp)
            active_dialog_step = "S3-1";
        else
            active_dialog_step = "S5-1";
    } else if (active == "menu_settings") {
        if (som_active && som_status == S1) {
            active_dialog = dialog_sequence::StartUp;
            active_dialog_step = "S1";
        } else {
            active_dialog = dialog_sequence::None;
        }
    } else if (active == "adhesion_window" || active == "sr_data_window") {
        active_dialog_step = "S1";
    } else if (active == "language_window" || active == "volume_window" || active == "brightness_window" || active == "system_version_window" || active == "set_vbc_window" || active == "set_vbc_validation_window" || active == "remove_vbc_window" || active == "remove_vbc_validation_window") {
        active_dialog_step = "S1";
    }
}
void validate_data_entry(std::string name, json &result)
{
    if (name != active_window_dmi["WindowDefinition"]["WindowTitle"])
        return;
    if (name == get_text("Level")) {
        std::string sel = result[""];
        Level lv = Level::Unknown;
        int nid_ntc = -1;
        if (sel == get_text("Level 0"))
            lv = Level::N0;
        else if (sel == get_text("Level 1"))
            lv = Level::N1;
        else if (sel == get_text("Level 2"))
            lv = Level::N2;
        else if (sel == get_text("Level 3"))
            lv = Level::N3;
        else for (auto &kvp : ntc_names) {
            if (kvp.second == sel) {
                lv = Level::NTC;
                nid_ntc = kvp.first;
                break;
            }
        }
        if (lv == Level::Unknown) return;
        driver_set_level({lv, nid_ntc});
        if (message_when_level_selected && !bot_driver) {
            int64_t time = get_milliseconds();
            add_message(text_message(get_text("Level selected"), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
        }
        if (active_dialog == dialog_sequence::Main) {
            if (level == Level::N2 || level == Level::N3) {
                active_dialog_step = "D5";
            } else {
                active_dialog_step = "S1";
            }
        }
    } else if (name == get_text("Train running number")) {
        train_running_number_valid = false;
        std::string res = result[""].get<std::string>();
        if (res.size() > 8)
            return;
        train_running_number = stoi(res);
        if (train_running_number > 0) {
            train_running_number_valid = true;
            if (message_when_running_number_entered && !bot_driver) {
                int64_t time = get_milliseconds();
                add_message(text_message(get_text("Train running number entered"), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
            }
        } else {
            return;
        }
        if (active_dialog == dialog_sequence::Main) {
            if (active_dialog_step == "S6") {
                active_dialog_step = "S1";
                if (supervising_rbc)
                    supervising_rbc->train_running_number_sent = false;
            } else
                active_dialog_step = "D1";
        } else if (active_dialog == dialog_sequence::StartUp) {
                active_dialog_step = "S1";
        }
    } else if (name == get_text("Train data")) {
        if (flexible_data_entry) {
            int length = stoi(result[get_text("Length (m)")].get<std::string>());
            std::string str = const_train_data.count("TrainCategory") ? get_text(const_train_data["TrainCategory"]) : result[get_text("Train category")].get<std::string>();
            int cat;
            if (str == get_text("PASS 1"))
                cat = 2;
            else if (str == get_text("PASS 2"))
                cat = 2;
            else if (str == get_text("PASS 3"))
                cat = 2;
            else if (str == get_text("TILT 1"))
                cat = 2;
            else if (str == get_text("TILT 2"))
                cat = 2;
            else if (str == get_text("TILT 3"))
                cat = 2;
            else if (str == get_text("TILT 4"))
                cat = 2;
            else if (str == get_text("TILT 5"))
                cat = 2;
            else if (str == get_text("TILT 6"))
                cat = 2;
            else if (str == get_text("TILT 7"))
                cat = 2;
            else if (str == get_text("FP 1"))
                cat = 0;
            else if (str == get_text("FP 2"))
                cat = 0;
            else if (str == get_text("FP 3"))
                cat = 0;
            else if (str == get_text("FP 4"))
                cat = 0;
            else if (str == get_text("FG 1"))
                cat = 1;
            else if (str == get_text("FG 2"))
                cat = 1;
            else if (str == get_text("FG 3"))
                cat = 1;
            else if (str == get_text("FG 4")) 
                cat = 1;
            // TODO: implement cross check
            if (length > 900 && cat == 2)
                return;
            if (length > 1500)
                return;
        }

        active_dialog_step = "S3-2";
        json j = R"({"active":"train_data_validation_window"})"_json;
        json def;
        def["WindowType"] = "DataValidation";
        def["WindowTitle"] = get_text("Validate train data");
        def["DataInputResult"] = result;
        j["WindowDefinition"] = def;
        active_window_dmi = j;
    } else if (name == get_text("Validate train data")) {
        if (!result["Validated"]) {
            prev_step = active_dialog_step = "S3-1";
            if (flexible_data_entry)
                active_window_dmi = train_data_window();
            else
                active_window_dmi = fixed_train_data_window();
            for (auto it = result.begin(); it != result.end(); ++it) {
                for (json &j : active_window_dmi["WindowDefinition"]["Inputs"]) {
                    if (j["Label"] == it.key())
                        j["Value"] = it.value();
                }
            }
            return;
        } else {
            if (flexible_data_entry) {
                train_data_valid = false;

                L_TRAIN = stoi(result[get_text("Length (m)")].get<std::string>());

                std::string gauge = const_train_data.count("LoadingGauge") ? get_text(const_train_data["LoadingGauge"]) : result[get_text("Loading gauge")].get<std::string>();
                if (gauge == get_text("G1"))
                    loading_gauge = loading_gauges::G1;
                else if (gauge == get_text("GA"))
                    loading_gauge = loading_gauges::GA;
                else if (gauge == get_text("GB"))
                    loading_gauge = loading_gauges::GB;
                else if (gauge == get_text("GC"))
                    loading_gauge = loading_gauges::GC;
                else
                    loading_gauge = loading_gauges::OutGC;

                std::string axlecat = const_train_data.count("AxleLoadCategory") ? get_text(const_train_data["AxleLoadCategory"]) : result[get_text("Axle load category")].get<std::string>();
                if (axlecat == get_text("A"))
                    axle_load_category = axle_load_categories::A;
                else if (axlecat == get_text("HS17"))
                    axle_load_category = axle_load_categories::HS17;
                else if (axlecat == get_text("B1"))
                    axle_load_category = axle_load_categories::B1;
                else if (axlecat == get_text("B2"))
                    axle_load_category = axle_load_categories::B2;
                else if (axlecat == get_text("C2"))
                    axle_load_category = axle_load_categories::C2;
                else if (axlecat == get_text("C3"))
                    axle_load_category = axle_load_categories::C3;
                else if (axlecat == get_text("C4"))
                    axle_load_category = axle_load_categories::C4;
                else if (axlecat == get_text("D2"))
                    axle_load_category = axle_load_categories::D2;
                else if (axlecat == get_text("D3"))
                    axle_load_category = axle_load_categories::D3;
                else if (axlecat == get_text("D4"))
                    axle_load_category = axle_load_categories::D4;
                else if (axlecat == get_text("D4XL"))
                    axle_load_category = axle_load_categories::D4XL;
                else if (axlecat == get_text("E4"))
                    axle_load_category = axle_load_categories::E4;
                else if (axlecat == get_text("E5"))
                    axle_load_category = axle_load_categories::E5;

                std::string air = const_train_data.count("Airtight") ? get_text(const_train_data["Airtight"]) : result[get_text("Airtight")].get<std::string>();
                Q_airtight = air == get_text("Yes");

                set_train_max_speed(stoi(result[get_text("Max speed (km/h)")].get<std::string>())/3.6);

                brake_percentage = stoi(result[get_text("Brake percentage")].get<std::string>());

                std::string str = const_train_data.count("TrainCategory") ? get_text(const_train_data["TrainCategory"]) : result[get_text("Train category")].get<std::string>();
                int cant;
                int cat;
                if (str == get_text("PASS 1")) {
                    cant = 80;
                    cat = 2;
                    train_category = "PASS 1";
                } else if (str == get_text("PASS 2")) {
                    cant = 130;
                    cat = 2;
                    train_category = "PASS 2";
                } else if (str == get_text("PASS 3")) {
                    cant = 150;
                    cat = 2;
                    train_category = "PASS 3";
                } else if (str == get_text("TILT 1")) {
                    cant = 165;
                    cat = 2;
                    train_category = "TILT 1";
                } else if (str == get_text("TILT 2")) {
                    cant = 180;
                    cat = 2;
                    train_category = "TILT 2";
                } else if (str == get_text("TILT 3")) {
                    cant = 210;
                    cat = 2;
                    train_category = "TILT 3";
                } else if (str == get_text("TILT 4")) {
                    cant = 225;
                    cat = 2;
                    train_category = "TILT 4";
                } else if (str == get_text("TILT 5")) {
                    cant = 245;
                    cat = 2;
                    train_category = "TILT 5";
                } else if (str == get_text("TILT 6")) {
                    cant = 275;
                    cat = 2;
                    train_category = "TILT 6";
                } else if (str == get_text("TILT 7")) {
                    cant = 300;
                    cat = 2;
                    train_category = "TILT 7";
                } else if (str == get_text("FP 1")) {
                    cant = 80;
                    cat = 0;
                    train_category = "FP 1";
                } else if (str == get_text("FP 2")) {
                    cant = 100;
                    cat = 0;
                    train_category = "FP 2";
                } else if (str == get_text("FP 3")) {
                    cant = 130;
                    cat = 0;
                    train_category = "FP 3";
                } else if (str == get_text("FP 4")) {
                    cant = 150;
                    cat = 0;
                    train_category = "FP 4";
                } else if (str == get_text("FG 1")) {
                    cant = 80;
                    cat = 1;
                    train_category = "FG 1";
                } else if (str == get_text("FG 2")) {
                    cant = 100;
                    cat = 1;
                    train_category = "FG 2";
                } else if (str == get_text("FG 3")) {
                    cant = 130;
                    cat = 1;
                    train_category = "FG 3";
                } else if (str == get_text("FG 4")) {
                    cant = 150;
                    cat = 1;
                    train_category = "FG 4";
                }
                train_data_known = true;
                cant_deficiency = cant;
                brake_position = (brake_position_types)cat;
                set_conversion_model();
                if (!conversion_model_used) {
                    // TODO: use pre-programmed deceleration curves if available
                } else {
                    train_data_valid = true;
                }
            } else {
                set_train_data(result[get_text("Train type")].get<std::string>());
            }
            if (message_when_train_data_entered && !bot_driver) {
                int64_t time = get_milliseconds();
                add_message(text_message(get_text("Train data entered"), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
            }
            train_shorten('j');
            if (train_data_valid) {
                active_dialog = dialog_sequence::NTCData;
                active_dialog_step = "D1";
                stm_send_train_data();
            } else {
                prev_step = active_dialog_step = "S3-1";
                if (flexible_data_entry)
                    active_window_dmi = train_data_window();
                else
                    active_window_dmi = fixed_train_data_window();
            }
        }
    } else if (name == get_text("Driver ID")) {
        driver_id = result[""];
        if (driver_id.length() < 0 || driver_id.length() > 16)
            return;
        driver_id_valid = true;
        if (active_dialog == dialog_sequence::StartUp)
            active_dialog_step = "D2";
        else if (active_dialog == dialog_sequence::Main)
            active_dialog_step = "S1";
        if (message_when_driver_id_entered && !bot_driver) {
            int64_t time = get_milliseconds();
            add_message(text_message(get_text("Driver ID entered"), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
        }
    } else if (name == get_text("RBC data")) {
        uint32_t id = atoll(result[get_text("RBC ID")].get<std::string>().c_str());
        uint64_t number = to_bcd(result[get_text("RBC phone number")].get<std::string>().c_str());
        set_supervising_rbc(contact_info({id>>14,id&((1<<14) - 1),number}));
        if (som_active && som_status == S3 ) {
            som_status = A31;
        } else {
            if (supervising_rbc)
                supervising_rbc->open(N_tries_radio);
            active_dialog_step = "S8";
        }
    } else if (name == get_text("Radio network ID")) {
        std::string id = result[""];
        if (id != "") {
            bool found=false;
            for (auto &kvp : RadioNetworkNames) {
                if (kvp.second == id) {
                    RadioNetworkId = kvp.first;
                    found = true;
                    break;
                }
            }
            if (!found) {
                RadioNetworkId = 100*stoi(id.substr(0,2))+stoi(id.substr(3));
            }
            for (mobile_terminal *t : mobile_terminals) {
                if (!t->registered || t->network_id != RadioNetworkId) {
                    t->network_register(RadioNetworkId);
                }
            }
            if (active_dialog == dialog_sequence::StartUp) {
                rbc_contact_valid = false;
                active_dialog_step = "S3-2-3";
            } else
                active_dialog_step = "S5-2-3";
        }
    } else if (name == get_text("Language")) {
        set_language(result[""]);
        if (active_dialog == dialog_sequence::Settings)
            active_dialog_step = "S1";
    } else if (name == get_text("SR speed/distance")) {
        if (mode == Mode::SR) {
            SR_speed_override = stod(result[get_text("SR speed (km/h)")].get<std::string>())/3.6;
            SR_speed = speed_restriction(*SR_speed_override, distance::from_odometer(dist_base::min), distance::from_odometer(dist_base::max), false);
            SR_dist_override = stod(result[get_text("SR distance (m)")].get<std::string>());
            SR_dist_start = distance::from_odometer(d_estfront_dir[odometer_orientation == -1]);
            recalculate_MRSP();
        }
        active_dialog_step = "S1";
    } else if (name == get_text("Adhesion")) {
        slippery_rail_driver = result[""] == get_text("Slippery rail");
        active_dialog_step = "S1";
    } else if (name == get_text("Set VBC")) {
        active_dialog_step = "S6-2";
        json j = R"({"active":"set_vbc_validation_window"})"_json;
        json def;
        def["WindowType"] = "DataValidation";
        def["WindowTitle"] = get_text("Validate set VBC");
        def["DataInputResult"] = result;
        j["WindowDefinition"] = def;
        active_window_dmi = j;
    } else if (name == get_text("Remove VBC")) {
        active_dialog_step = "S7-2";
        json j = R"({"active":"remove_vbc_validation_window"})"_json;
        json def;
        def["WindowType"] = "DataValidation";
        def["WindowTitle"] = get_text("Validate remove VBC");
        def["DataInputResult"] = result;
        j["WindowDefinition"] = def;
        active_window_dmi = j;
    } else if (name == get_text("Validate set VBC")) {
        if (!result["Validated"]) {
            prev_step = active_dialog_step = "S6-1";
            active_window_dmi = set_vbc_window();
            for (auto it = result.begin(); it != result.end(); ++it) {
                for (json &j : active_window_dmi["WindowDefinition"]["Inputs"]) {
                    if (j["Label"] == it.key())
                        j["Value"] = it.value();
                }
            }
            return;
        } else {
            std::string t = get_text("VBC code");
            uint32_t num = stoi(result[get_text("VBC code")].get<std::string>());
            set_vbc({(int)(num>>6) & 1023, (int)(num & 63), get_milliseconds(), (num>>16)*86400000LL});
            active_dialog_step = "S1";
        }
    } else if (name == get_text("Validate remove VBC")) {
        if (!result["Validated"]) {
            prev_step = active_dialog_step = "S7-1";
            active_window_dmi = remove_vbc_window();
            for (auto it = result.begin(); it != result.end(); ++it) {
                for (json &j : active_window_dmi["WindowDefinition"]["Inputs"]) {
                    if (j["Label"] == it.key())
                        j["Value"] = it.value();
                }
            }
            return;
        } else {
            uint32_t num = stoi(result[get_text("VBC code")].get<std::string>());
            remove_vbc({(int)(num>>6) & 1023, (int)(num & 63), 0, 0});
            active_dialog_step = "S1";
        }
    } else if (name == get_text("Brightness")) {
        active_dialog_step = "S1";
    } else if (name == get_text("Volume")) {
        active_dialog_step = "S1";
    } else {
        for (auto &kvp : installed_stms) {
            auto *stm = kvp.second;
            if (stm->data_entry == stm_object::data_entry_state::Driver) {
                if ((name == get_ntc_name(kvp.first)+get_text(" data"))) {
                    active_dialog_step = "S3-2";
                    json j = R"({"active":"ntc_data_validation_window"})"_json;
                    j["ntc"] = get_ntc_name(kvp.first);
                    json def;
                    def["WindowType"] = "DataValidation";
                    def["WindowTitle"] = get_text("Validate ")+get_ntc_name(kvp.first)+get_text(" data");
                    def["DataInputResult"] = result;
                    j["WindowDefinition"] = def;
                    active_window_dmi = j;
                } else if ((name == get_text("Validate ")+get_ntc_name(kvp.first)+get_text(" data"))) {
                    if (!result["Validated"]) {
                        prev_step = active_dialog_step = "S3-1";
                        active_window_dmi = ntc_data_window();
                        for (auto it = result.begin(); it != result.end(); ++it) {
                            for (json &j : active_window_dmi["WindowDefinition"]["Inputs"]) {
                                if (j["Label"] == it.key())
                                    j["Value"] = it.value();
                            }
                        }
                        return;
                    } else {
                        stm->send_specific_data(result);
                        active_dialog_step = "S4";
                    }
                }
            }
        }
    }
}
void validate_entry_field(std::string window, json &result)
{
    bool operat = true;
    bool techres = true;
    bool techrang = true;
    std::string label = result["Label"];
    std::string data = result["Value"];
    if (window == get_text("Train data")) {
        if (label == get_text("Length (m)")) {
            int val = atoi(data.c_str());
            techrang = val >= 0 && val < 4096;
            operat = val < 1500;
        } else if (label == get_text("Brake percentage")) {
            int val = atoi(data.c_str());
            techrang = val >= 10 && val <= 250;
            operat = val >= 30;
        } else if (label == get_text("Max speed (km/h)")) {
            int val = atoi(data.c_str());
            techrang = val > 0 && val <= 600;
            techres = val % 5 == 0;
            operat = val <= 200;
        }
    } else if (window == get_text("RBC data")) {
        if (label == get_text("RBC ID")) {
            int val = atoi(data.c_str());
            techrang = val >= 0 && val <= 16777214;
        }
    } else if (window == get_text("SR speed/distance")) {
        if (label == get_text("SR distance (m)")) {
            int val = atoi(data.c_str());
            techrang = val > 0 && val <= 100000;
        } else if (label == get_text("SR speed (km/h)")) {
            int val = atoi(data.c_str());
            techrang = val > 0 && val <= 600;
            techres = val % 5 == 0;
        }
    }
    result["OperationalRange"] = operat || result["SkipOperationalCheck"];
    result["TechnicalResolution"] = techres;
    result["TechnicalRange"] = techrang;
    json j;
    j["ValidateEntryField"] = result;
    send_command("json", j.dump());
}
void update_dialog_step(std::string step, std::string step2)
{
    dialog_sequence prev_seq = active_dialog;
    std::string prev_step = active_dialog_step;
    if (active_dialog == dialog_sequence::None) {
        if (step2 == "main") {
            active_dialog_step = "S1";
            active_dialog = dialog_sequence::Main;
        } else if (step2 == "override") {
            active_dialog = dialog_sequence::Override;
            active_dialog_step = "S1";
        } else if (step2 == "data_view") {
            active_dialog = dialog_sequence::DataView;
        } else if (step2 == "spec") {
            active_dialog = dialog_sequence::Special;
            active_dialog_step = "S1";
        } else if (step2 == "settings") {
            active_dialog = dialog_sequence::Settings;
            active_dialog_step = "S1";
        }
    } else if (active_dialog == dialog_sequence::StartUp) {
        if (step2 == "settings") {
            active_dialog_step = "S1-1";
        } else if (step == "TrainRunningNumber") {
            active_dialog_step = "S1-2";
        } else if (step == "ContactLastRBC" || step == "UseShortNumber") {
            set_supervising_rbc(step == "ContactLastRBC" ? contact_info({0,ContactLastRBC,0}) : contact_info({0,0,UseShortNumber}));
            som_status = A31;
        } else if (step == "EnterRBCdata") {
            active_dialog_step = "S3-3";
        } else if (step == "RadioNetworkID") {
            for (auto *session : active_sessions) {
                if (session->status != session_status::Inactive)
                    session->close();
            }
            retrieve_radio_networks();
            active_dialog_step = "S3-2-1";
        }
    } else if (active_dialog == dialog_sequence::Main) {
        if (step == "Start") {
            start_pressed();
            if (level == Level::N2 || level == Level::N3) {
                active_dialog_step = "D7";
            } else {
                active_dialog = dialog_sequence::None;
            }
        } else if (step == "Level") {
            active_dialog_step = "S4";
        } else if (step == "RadioData") {
            active_dialog_step = "S5-1";
        } else if (step == "DriverID") {
            active_dialog_step = "S2";
        } else if (step == "TrainRunningNumber") {
            active_dialog_step = "S6";
        } else if (step == "TrainData") {
            active_dialog_step = "S3-1";
        } else if (step == "SelectType") {
            if (data_entry_type == 2) {
                flexible_data_entry = false;
                active_window_dmi = fixed_train_data_window();
            }
        } else if (step == "EnterData") {
            if (data_entry_type == 2) {
                flexible_data_entry = true;
                active_window_dmi = train_data_window();
            }
        } else if (step == "Shunting") {
            if (V_est == 0 && mode == Mode::SH) {
                trigger_condition(19);
            }
            if (V_est == 0 && (level==Level::N0 || level==Level::NTC || level==Level::N1) && mode != Mode ::SH) {
                if (level == Level::NTC) {
                    auto *stm = get_stm(nid_ntc);
                    if (stm != nullptr && stm->national_trip)
                        trigger_condition(35);
                    else
                        trigger_condition(5);
                } else {
                    trigger_condition(5);
                }
            }
            if (V_est == 0 && (level == Level::N2 || level == Level::N3)) {
                if (supervising_rbc && supervising_rbc->status == session_status::Established && emergency_stops.empty()) {
                    supervising_rbc->queue(std::make_shared<SH_request>());
                }
            }
            if (mode == Mode::SH) {
                active_dialog = dialog_sequence::None;
            } else {
                active_dialog = dialog_sequence::Shunting;
                active_dialog_step = "D1";
            }
        } else if (step == "NonLeading") {
            active_dialog = dialog_sequence::None;
            if (V_est == 0 && nl_signal)
                trigger_condition(46);
        } else if (step == "MaintainShunting") {
            active_dialog = dialog_sequence::None;
        } else if (step == "ContactLastRBC" || step == "UseShortNumber") {
            set_supervising_rbc(step == "ContactLastRBC" ? contact_info({0,ContactLastRBC,0}) : contact_info({0,0,UseShortNumber}));
            if (supervising_rbc)
                supervising_rbc->open(N_tries_radio);
            active_dialog_step = "S8";
        } else if (step == "EnterRBCdata") {
            active_dialog_step = "S5-3";
        } else if (step == "RadioNetworkID") {
            retrieve_radio_networks();
            active_dialog_step = "S5-2-1";
        }
    } else if (active_dialog == dialog_sequence::NTCData) {
        if (active_window_dmi["active"] != "menu_ntc")
            return;
        if (step == "EndDataEntry" && active_window_dmi["enabled"][step]) {
            active_dialog = dialog_sequence::Main;
            active_dialog_step = "D6";
        } else if (step == "STM" && active_window_dmi["enabled"].contains(step2) && active_window_dmi["enabled"][step2]) {
            for (auto &kvp : installed_stms) {
                auto *stm = kvp.second;
                if (step2 == get_ntc_name(kvp.first) && stm->data_entry == stm_object::data_entry_state::Active) {
                    stm->data_entry = stm_object::data_entry_state::Driver;
                    active_dialog_step = "S3-1";
                }
            }
        }
    } else if (active_dialog == dialog_sequence::Override) {
        if (step == "Override" && active_window_dmi["enabled"]["EoA"]) {
            start_override();
            active_dialog = dialog_sequence::None;
        }
    } else if (active_dialog == dialog_sequence::Shunting) {
        if (step == "SH refused") {
            add_message(text_message(get_text("SH refused"), true, false, 0, [](text_message &t){return any_button_pressed;}));
            active_dialog = dialog_sequence::Main;
            active_dialog_step = "S1";
            return;
        } else if (step == "SH authorised") {
            active_dialog = dialog_sequence::None;
            if (V_est == 0 && (level == Level::N2 || level == Level::N3))
                trigger_condition(6);
            return;
        }
    } else if (active_dialog == dialog_sequence::Special) {
        if (active_window_dmi["active"] != "menu_spec" || !active_window_dmi["enabled"].contains(step) || !active_window_dmi["enabled"][step])
            return;
        if (step == "Adhesion") {
            active_dialog_step = "S2";
        } else if (step == "SRspeed") {
            active_dialog_step = "S3";
        } else if (step == "TrainIntegrity") {
            active_dialog = dialog_sequence::None;
        }
    } else if (active_dialog == dialog_sequence::Settings) {
        if (active_window_dmi["active"] != "menu_settings" || !active_window_dmi["enabled"].contains(step) || !active_window_dmi["enabled"][step])
            return;
        if (step == "Language")
            active_dialog_step = "S2";
        else if (step == "Volume")
            active_dialog_step = "S3";
        else if (step == "Brightness")
            active_dialog_step = "S4";
        else if (step == "SystemVersion")
            active_dialog_step = "S5";
        else if (step == "SetVBC")
            active_dialog_step = "S6-1";
        else if (step == "RemoveVBC")
            active_dialog_step = "S7-1";
    }
    if (active_dialog != prev_seq || active_dialog_step != prev_step) {
        any_button_pressed_async = true;
    }
}
