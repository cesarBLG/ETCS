#include "windows.h"
#include "../Supervision/emergency_stop.h"
#include "../Supervision/supervision.h"
#include "../Euroradio/session.h"
#include "../Procedures/start.h"
#include "../Procedures/override.h"
#include "../Procedures/train_trip.h"
#include <fstream>
dialog_sequence active_dialog;
std::string active_dialog_step;
json default_window = R"({"active":"default"})"_json;
json active_window_dmi = default_window;
const json main_window_radio_wait = R"({"active":"menu_main","hour_glass":true,"enabled":{"Start":false,"Driver ID":false,"Train Data":false,"Level":false,"Train Running Number":false,"Maintain Shunting":false,"Shunting":false,"Non Leading":false,"Radio Data":false,"Exit":false}})"_json;
const json radio_window_radio_wait = R"({"active":"menu_radio","hour_glass":true,"enabled":{"Exit":false}})"_json;
bool pending_train_data_send = false;
bool any_button_pressed_async = false;
bool any_button_pressed = false;
/*json fixed_train_data_window()
{
    json &def = j["WindowDefinition"];
    def["WindowTitle"] = "Train Data";
    def["WindowType"] = "DataEntry";
    json &data = def["Inputs"][0];
    data["Label"] = "";
    data["AcceptedValue"] = special_train_data;
    data["Keyboard"]["Type"] = "Dedicated";

#ifdef __ANDROID__
    extern std::string filesDir;
    std::ifstream file(filesDir+"/traindata.txt");
#else
    std::ifstream file("traindata.txt");
#endif
    json td;
    file >> td;
    std::vector<std::string> types;
    for (auto it = td.begin(); it!=td.end(); ++it) {
        types.push_back(it.key());
    }
    data["Keyboard"]["Keys"] = types;
    return j;
}*/
json driver_id_window(bool trn)
{
    json j = R"({"active":"driver_window"})"_json;
    j["driver_id"] = driver_id;
    j["show_trn"] = trn;
    return j;
}
void update_dmi_windows()
{
    any_button_pressed = any_button_pressed_async;
    any_button_pressed_async = false;
    json prev_active = active_window_dmi;
    if (active_dialog == dialog_sequence::None) {
        active_window_dmi = default_window;
    } else if (active_dialog == dialog_sequence::StartUp) {
        if (!som_active)
            active_dialog = dialog_sequence::None;
        if (active_dialog_step == "S0") {
            active_window_dmi = main_window_radio_wait;
            if (som_status != S0)
                active_dialog_step = "S1";
        } else if (active_dialog_step == "S1") {
            active_window_dmi = driver_id_window(true);
            if (som_status != S1)
                active_dialog_step = "D2";
        } else if (active_dialog_step == "S1-1") {
            active_window_dmi = R"({"active":"trn_window"})"_json;
            active_window_dmi["trn"] = train_running_number;
        } else if (active_dialog_step == "S1-2") {
        } else if (active_dialog_step == "S2") {
            active_window_dmi = R"({"active":"level_window"})"_json;
            active_window_dmi["level"] = (int)level;
            if (som_status != S2) {
                if (level == Level::N2 || level == Level::N3)
                    active_dialog_step = "S3-1";
                else
                    active_dialog_step = "S10";
            }
        } else if (active_dialog_step == "S3-1") {
            active_window_dmi = R"({"active":"menu_radio"})"_json;
            if (som_status != S3)
                active_dialog_step = "A31";
        } else if (active_dialog_step == "S3-2-1") {
            active_window_dmi = radio_window_radio_wait;
        } else if (active_dialog_step == "S3-2-2") {
        } else if (active_dialog_step == "S3-2-3") {
            active_window_dmi = radio_window_radio_wait;
        } else if (active_dialog_step == "S3-3") {
        } else if (active_dialog_step == "S4") {
            active_window_dmi = main_window_radio_wait;
        } else if (active_dialog_step == "A29") {
            active_dialog_step = "S10";
        } else if (active_dialog_step == "A31") {
            active_window_dmi = main_window_radio_wait;
            if (som_status != A31)
                active_dialog_step = "D31";
        } else if (active_dialog_step == "A32") {
            active_dialog_step = "S10";
        } else if (active_dialog_step == "A40") {
            add_message(text_message("Train is rejected", true, false, 0, [](text_message &t){return any_button_pressed;})); // TODO
            active_dialog_step = "S10";
        } else if (active_dialog_step == "D2") {
            if (level_valid)
                active_dialog_step = "D3";
            else
                active_dialog_step = "S2";
        } else if (active_dialog_step == "D3") {
            if (level == Level::N2 || level == Level::N3)
                active_dialog_step = "D7";
            else
                active_dialog_step = "S10";
        } else if (active_dialog_step == "D7") {
            bool registered = false;
            for (mobile_terminal &t : mobile_terminals) {
                if (t.registered) {
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
            active_window_dmi = R"({"active":"menu_main"})"_json;
        } else if (active_dialog_step == "S2") {
            active_window_dmi = driver_id_window(false);
        } else if (active_dialog_step == "S3-1") {
            active_window_dmi = R"({"active":"fixed_train_data_window"})"_json;;
            active_window_dmi["train_data"] = special_train_data;
        } else if (active_dialog_step == "S3-2") {
            //active_window_dmi = fixed_train_data_validation_window();
        } else if (active_dialog_step == "S3-3") {
            active_window_dmi = R"({"active":"trn_window"})"_json;
            active_window_dmi["trn"] = train_running_number;
        } else if (active_dialog_step == "S4") {
            active_window_dmi = R"({"active":"level_window"})"_json;
            active_window_dmi["level"] = (int)level;
        } else if (active_dialog_step == "S5-1") {
            active_window_dmi = R"({"active":"menu_radio"})"_json;
        } else if (active_dialog_step == "S5-2-1") {
            active_window_dmi = radio_window_radio_wait;
        } else if (active_dialog_step == "S5-2-2") {
        } else if (active_dialog_step == "S5-2-3") {
            active_window_dmi = radio_window_radio_wait;
        } else if (active_dialog_step == "S5-3") {
        } else if (active_dialog_step == "S6") {
            active_window_dmi = R"({"active":"trn_window"})"_json;
            active_window_dmi["trn"] = train_running_number;
        } else if (active_dialog_step == "S7") {
            active_window_dmi = main_window_radio_wait;
            if (!supervising_rbc || supervising_rbc->status == session_status::Inactive)
                active_dialog_step = "S1";
        } else if (active_dialog_step == "S8") {
            active_window_dmi = main_window_radio_wait;
            if (!supervising_rbc || supervising_rbc->status != session_status::Establishing)
                active_dialog_step = "D3";
        } else if (active_dialog_step == "S9") {
            active_window_dmi = main_window_radio_wait;
            if (!supervising_rbc || supervising_rbc->status == session_status::Inactive || !supervising_rbc->train_data_ack_pending)
                active_dialog_step = "S1";
        } else if (active_dialog_step == "D1") {
            if (level == Level::N2 || level == Level::N3)
                active_dialog_step = "D2";
            else
                active_dialog_step = "S1";
        } else if (active_dialog_step == "D2") {
            if (supervising_rbc && supervising_rbc->status == session_status::Established) {
                active_dialog_step = "S9";
                supervising_rbc->train_data_ack_pending = true;
                supervising_rbc->train_data_ack_sent = false;
            } else {
                active_dialog_step = "S1";
            }
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
            if (rbc_contact && rbc_contact_valid) {
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
    } else if (active_dialog == dialog_sequence::Override) {
        if (active_dialog_step == "S1")
            active_window_dmi = R"({"active":"menu_override"})"_json;
    } else if (active_dialog == dialog_sequence::Shunting) {
        if (active_dialog_step == "D1") {
            if (level == Level::N2 || level == Level::N3) {
                active_dialog_step = "S1";
            } else/* if (level == level::N0 || level == level::N1) */{
                active_dialog = dialog_sequence::None;
            }
        } else if (active_dialog_step == "S1") {
            active_window_dmi = main_window_radio_wait;
            if (!supervising_rbc || supervising_rbc->status != session_status::Established) {
                active_dialog = dialog_sequence::Main;
                active_dialog_step = "S1";
                add_message(text_message("Shunting request failed", true, false, 0, [](text_message &t){return any_button_pressed;}));
            }
        }
    }
    std::string active = active_window_dmi["active"];
    if (active == "menu_main" && !active_window_dmi.contains("hour_glass")) {
        json &enabled = active_window_dmi["enabled"];
        bool c1 = V_est == 0 && mode == Mode::SB && train_data_valid && level != Level::Unknown;
        bool c2 = V_est == 0 && mode == Mode::PT && train_data_valid && (level == Level::N1 || ((level == Level::N2 || level == Level::N3) && trip_exit_acknowledged && supervising_rbc && supervising_rbc->status == session_status::Established && emergency_stops.empty()));
        bool c3 = mode == Mode::SR && (level == Level::N2 || level == Level::N3) && supervising_rbc && supervising_rbc->status == session_status::Established;
        enabled["Start"] = c1 || c2 || c3;
        enabled["Driver ID"] = (V_est == 0 && mode == Mode::SB && driver_id_valid && level_valid) || ((M_NVDERUN || (!M_NVDERUN && V_est == 0)) &&
            (mode == Mode::SH || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::UN || mode == Mode::SN));
        enabled["Train Data"] = V_est == 0 && driver_id_valid && level_valid &&
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::UN || mode == Mode::SN);
        enabled["Maintain Shunting"] = mode == Mode::SH && false;
        enabled["Level"] = V_est == 0 && driver_id_valid &&
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::UN || mode == Mode::SN);
        enabled["Train Running Number"] = (V_est == 0 && mode == Mode::SB && driver_id_valid && level_valid) ||
            (mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::UN || mode == Mode::SN);
        enabled["Shunting"] = (mode == Mode::SH && V_est == 0) ||
            (V_est == 0 && driver_id_valid && (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::UN || mode == Mode::SN)
                && level_valid && (level == Level::N0 || level == Level::N1 || level == Level::NTC || ((level == Level::N2 || level == Level::N3) && supervising_rbc && supervising_rbc->status == session_status::Established))) ||
            (V_est == 0 && mode == Mode::PT && (level == Level::N1 || ((level == Level::N2 || level == Level::N3) && trip_exit_acknowledged && supervising_rbc && supervising_rbc->status == session_status::Established && !emergency_stops.empty())));
        enabled["Non Leading"] = false;
        enabled["Radio Data"] = V_est == 0 && driver_id_valid && level_valid &&
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::PT || mode == Mode::UN || mode == Mode::SN);;
    } else if (active == "menu_radio" && !active_window_dmi.contains("hour_glass")) {
        json &enabled = active_window_dmi["enabled"];
        bool registered = false;
        for (mobile_terminal &t : mobile_terminals) {
            if (t.registered) {
                registered = true;
                break;
            }
        }
        enabled["Contact last RBC"] = V_est == 0 && driver_id_valid && 
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::PT) && 
            level_valid && (level == Level::N2 || level == Level::N3) && registered && rbc_contact;
        enabled["Use short number"] = V_est == 0 && driver_id_valid && 
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::PT) && 
            level_valid && (level == Level::N2 || level == Level::N3) && registered;
        enabled["Enter RBC data"] = V_est == 0 && driver_id_valid && 
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::PT) && 
            level_valid && (level == Level::N2 || level == Level::N3) && registered;
        enabled["Radio Network ID"] = V_est == 0 && driver_id_valid && 
            (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::PT) && 
            level_valid;
    } else if (active == "menu_override") {
        active_window_dmi["enabled"]["EoA"] = V_est <= V_NVALLOWOVTRP && (((mode == Mode::FS || mode == Mode::OS || mode == Mode::LS || mode == Mode::SR || mode == Mode::UN || mode == Mode::PT || mode == Mode::SB || mode == Mode::SN) && train_data_valid) || mode == Mode::SH);
    }
}
void close_window()
{
    std::string active = active_window_dmi["active"];
    if (active == "menu_main" || active == "menu_override")
        active_dialog = dialog_sequence::None;
    else if (active == "trn_window") {
        if (active_dialog_step == "S3-3" && supervising_rbc) {
            supervising_rbc->train_data_ack_pending = true;
            supervising_rbc->train_data_ack_sent = false;
        }
        active_dialog_step = "S1";   
    } else if (active == "fixed_train_data_window" || active == "level_window" || active == "fixed_train_data_validation_window")
        active_dialog_step = "S1";
    else if (active == "menu_radio")
        active_dialog_step = "S1";
}
void validate_data_entry(std::string name, json &result)
{
    if (name == "Level") {
        level = (Level)result["Level"].get<int>();
        position_report_reasons[6] = 2;
        level_valid = true;
    } else if (name == "TrainRunningNumber") {

    }
}
void update_dialog_step(std::string step, std::string step2)
{
    dialog_sequence prev_seq = active_dialog;
    std::string prev_step = active_dialog_step;
    if (step == "setLevel") {
        level_valid = false;
        level = Level::Unknown;
        if (step2 == "0" || step2 == "1" || step2 == "2" || step2 == "3") {
            level = (Level)stoi(step2);
            level_valid = true;
        }
        position_report_reasons[6] = 2;
    } else if (step == "setTRN") {
        train_running_number_valid = false;
        train_running_number = stoi(step2);
        if (train_running_number > 0) {
            train_running_number_valid = true;
        }
    } else if (step == "setDriverID") {
        driver_id = step2;
        driver_id_valid = true;
    }
    if (active_dialog == dialog_sequence::None) {
        if (step2 == "main") {
            active_dialog_step = "S1";
            active_dialog = dialog_sequence::Main;
        } else if (step2 == "override") {
            active_dialog = dialog_sequence::Override;
            active_dialog_step = "S1";
        }
    } else if (active_dialog == dialog_sequence::StartUp) {
        if (step == "setDriverID") {
            active_dialog_step = "D2";
        } else if (step == "TrainRunningNumber") {
            active_dialog_step = "S1-1";
        } else if (step == "setTRN") {
            active_dialog_step = "S1";
        } else if (step == "ContactLastRBC" || step == "UseShortNumber") {
            set_supervising_rbc(step == "ContactLastRBC" ? contact_info({0,NID_RBC_t::ContactLastRBC,0}) : contact_info({0,0,NID_RADIO_t::UseShortNumber}));
            som_status = A31;
        }
    } else if (active_dialog == dialog_sequence::Main) {
        if (step == "Start") {
            start_pressed();
            if ((level == Level::N2 || level == Level::N3) && supervising_rbc && supervising_rbc->status == session_status::Established) {
                active_dialog_step = "S7";
            } else {
                active_dialog = dialog_sequence::None;
            }
        } else if (step == "setLevel") {
            if (level == Level::N2 || level == Level::N3) {
                active_dialog_step = "D5";
            } else {
                active_dialog_step = "S1";
            }
        } else if (step == "setDriverID") {
            active_dialog_step = "S1";
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
        } else if (step == "Shunting") {
            if (V_est == 0 && mode == Mode::SH) {
                trigger_condition(19);
            }
            if (V_est == 0 && (level==Level::N0 || level==Level::NTC || level==Level::N1) && mode != Mode ::SH) {
                trigger_condition(5);
            }
            if (V_est == 0 && (level == Level::N2 || level == Level::N3)) {
                if (supervising_rbc && supervising_rbc->status == session_status::Established && emergency_stops.empty()) {
                    SH_request *req = new SH_request();
                    fill_message(req);
                    supervising_rbc->send(std::shared_ptr<SH_request>(req));
                }
            }
            if (mode == Mode::SH) {
                active_dialog = dialog_sequence::None;
            } else {
                active_dialog = dialog_sequence::Shunting;
                active_dialog_step = "D1";
            }
        } else if (step == "MaintainShunting" || step == "NonLeading") {
            active_dialog = dialog_sequence::None;
        } else if (step == "ContactLastRBC" || step == "UseShortNumber") {
            set_supervising_rbc(step == "ContactLastRBC" ? contact_info({0,NID_RBC_t::ContactLastRBC,0}) : contact_info({0,0,NID_RADIO_t::UseShortNumber}));
            if (supervising_rbc)
                supervising_rbc->open(N_tries_radio);
            active_dialog_step = "S8";
        } else if (step == "setAcceptedTrainData") {
            active_dialog_step = "S3-2";
            active_window_dmi = R"({"active":"fixed_train_data_validation_window"})"_json;
            active_window_dmi["train_data"] = step2;
        } else if (step == "setTrainData") {
            if (step2 == "") {
                active_dialog_step = "S3-1";
            } else {
                active_dialog_step = "D6";
                set_train_data(step2);
            }
        } else if (step == "setTRN") {
            if (active_dialog_step == "S6")
                active_dialog_step = "S1";
            else
                active_dialog_step = "D1";
        }
    } else if (active_dialog == dialog_sequence::Override) {
        if (step == "Override") {
            start_override();
            active_dialog = dialog_sequence::None;
        }
    } else if (active_dialog == dialog_sequence::Shunting) {
        if (step == "SH refused") {
            add_message(text_message("SH refused", true, false, 0, [](text_message &t){return any_button_pressed;}));
            active_dialog = dialog_sequence::Main;
            active_dialog_step = "S1";
        } else if (step == "SH authorised") {
            active_dialog = dialog_sequence::None;
            if (V_est == 0 && (level == Level::N2 || level == Level::N3))
                trigger_condition(6);
        }
    }
    if (active_dialog != prev_seq || active_dialog_step != prev_step) {
        any_button_pressed_async = true;
    }
}