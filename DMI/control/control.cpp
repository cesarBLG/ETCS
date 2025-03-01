/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "control.h"
#include "../graphics/display.h"
#include "../graphics/drawing.h"
#include "../monitor.h"
#include "../planning/planning.h"
#include "../window/window.h"
#include "../window/data_view.h"
#include "../window/train_data.h"
#include "../window/fixed_train_data.h"
#include "../window/driver_id.h"
#include "../window/volume.h"
#include "../window/brightness.h"
#include "../window/menu_main.h"
#include "../window/menu_radio.h"
#include "../window/menu_override.h"
#include "../window/menu_spec.h"
#include "../window/menu_settings.h"
#include "../window/menu_ntc.h"
#include "../window/track_ahead_free.h"
#include "../speed/gauge.h"
#include <functional>
subwindow *active = nullptr;
std::string active_name;
window *default_window;
void setupMessages();
void setupTimeHour();
void setupGpsPos();
void startWindows()
{
    setupMessages();
    setupTimeHour();
    setupGpsPos();

    prevMaxSpeed = 0;
    active_windows.clear();
    etcs_default_window.construct();
    default_window = &etcs_default_window;
    extern int maxSpeed;
    extern int etcsDialMaxSpeed;
    maxSpeed = etcsDialMaxSpeed;
    active_windows.push_front(default_window);
    active_name = "default";
}
void setWindow(json &data)
{
    if (data.contains("ValidateEntryField"))
    {
        if (active != nullptr)
        {
            ((input_window*)active)->fieldCheckResult(data["ValidateEntryField"]);
        }
    }
    if (!data.contains("ActiveWindow")) return;
    json j = data["ActiveWindow"];
    subwindow *w = nullptr;
    std::string name = j["active"].get<std::string>();
    if (name == "default")
    {
        extern bool showSpeeds;
        setPlanning(!display_taf && (mode == Mode::FS || (mode == Mode::OS && showSpeeds)));
        setTAF(display_taf);
    }
    else
    {
        bool same = name == active_name;
        if (name == "menu_main") {
            menu_main *m;
            if (same) m = (menu_main*)active;
            else m = new menu_main();
            json &enabled = j["enabled"];
            m->setEnabled(enabled["Start"].get<bool>(),enabled["Driver ID"].get<bool>(),enabled["Train Data"].get<bool>(),enabled["Level"].get<bool>(),enabled["Train Running Number"].get<bool>(),enabled["Shunting"].get<bool>(),enabled["Non Leading"].get<bool>(),enabled["Maintain Shunting"].get<bool>(),enabled["Radio Data"].get<bool>());
            m->setHourGlass(j.contains("hour_glass") && j["hour_glass"].get<bool>());
            w = m;
        } else if (name == "menu_radio") {
            menu_radio *m;
            if (same) m = (menu_radio*)active;
            else m = new menu_radio();
            json &enabled = j["enabled"];
            m->setEnabled(enabled["Contact last RBC"].get<bool>(),enabled["Use short number"].get<bool>(),enabled["Enter RBC data"].get<bool>(),enabled["Radio Network ID"].get<bool>());
            m->setHourGlass(j.contains("hour_glass") && j["hour_glass"].get<bool>());
            w = m;
        } else if (name == "menu_override") {
            menu_override *m;
            if (same) m = (menu_override*)active;
            else m = new menu_override();
            json &enabled = j["enabled"];
            m->setEnabled(enabled["EoA"].get<bool>());
            w = m;
        } else if (name == "menu_spec") {
            menu_spec *m;
            if (same) m = (menu_spec*)active;
            else m = new menu_spec();
            json &enabled = j["enabled"];
            m->setEnabled(enabled["Adhesion"].get<bool>(), enabled["SRspeed"].get<bool>(), enabled["TrainIntegrity"].get<bool>());
            w = m;
        } else if (name == "menu_settings") {
            menu_settings *m;
            if (same) m = (menu_settings*)active;
            else m = new menu_settings();
            json &enabled = j["enabled"];
            m->setEnabled(enabled["Language"].get<bool>(), enabled["Volume"].get<bool>(), enabled["Brightness"].get<bool>(), enabled["SystemVersion"].get<bool>(), enabled["SetVBC"].get<bool>(), enabled["RemoveVBC"].get<bool>());
            w = m;
        } else if (name == "menu_ntc") {
            menu_ntc *m;
            if (same) m = (menu_ntc*)active;
            else m = new menu_ntc(j["STMs"]);
            bool hour = j.contains("hour_glass") && j["hour_glass"].get<bool>();
            m->setHourGlass(hour);
            m->setEnabled(j["enabled"]);
            w = m;
        } else if (name == "driver_window") {
            driver_window *d;
            if (same) d = (driver_window*)active;
            else
            {
                json& def = j["WindowDefinition"];
                d = new driver_window(def["WindowTitle"].get<std::string>(), j["show_trn"].get<bool>());
                ((input_window*)d)->buildFrom(def);
            }
            w = d;
        } else if (name == "train_data_window") {
            train_data_window *t;
            if (same) t = (train_data_window*)active;
            else
            {
                json& def = j["WindowDefinition"];
                t = new train_data_window(def["WindowTitle"].get<std::string>(), j["Switchable"].get<bool>(), def["Inputs"].size());
                ((input_window*)t)->buildFrom(def);
            }
            w = t;
        } else if (name == "fixed_train_data_window") {
            fixed_train_data_window *t;
            if (same) t = (fixed_train_data_window*)active;
            else
            {
                json& def = j["WindowDefinition"];
                t = new fixed_train_data_window(def["WindowTitle"].get<std::string>(), j["Switchable"].get<bool>());
                ((input_window*)t)->buildFrom(def);
            }
            w = t;
        } else if (name == "volume_window") {
            volume_window *v;
            if (same) v = (volume_window*)active;
            else v = new volume_window();
            w = v;
        } else if (name == "brightness_window") {
            brightness_window *v;
            if (same) v = (brightness_window*)active;
            else v = new brightness_window();
            w = v;
        } else {
            if (same) w = active;
            else
            {
                json& def = j["WindowDefinition"];
                std::string type = def["WindowType"].get<std::string>();
                if (type == "DataEntry")
                {
                    w = new input_window(def["WindowTitle"].get<std::string>(), def["Inputs"].size(), def["Inputs"][0]["Label"] != "");
                    ((input_window*)w)->buildFrom(def);
                }
                else if (type == "DataValidation")
                {
                    std::vector<input_data*> data;
                    auto res = def["DataInputResult"];
                    for (auto it = res.begin(); it != res.end(); ++it)
                    {
                        input_data *i = new input_data(it.key());
                        i->data = it.value().get<std::string>();
                        i->setAccepted(true);
                        data.push_back(i);
                    }
                    w = new validation_window(def["WindowTitle"].get<std::string>(), data);
                }
                else if (type == "DataView")
                {
                    w = new data_view_window(def["WindowTitle"], def["Fields"]);
                }
                /*else if (type == "Menu") w = new menu(j["WindowDefinitionW"]);*/
            }
        }
        if (w != nullptr)
        {
            w->exit_button.enabled = !j.contains("enabled") || !j["enabled"].contains("Exit") || j["enabled"]["Exit"].get<bool>();
        }
        setPlanning(false);
        setTAF(false);
    }
    active_name = name;
    if (active != w) {
        if (active != nullptr) {
            for (auto it = active_windows.begin(); it != active_windows.end(); )
            {
                if (*it == active)
                {
                    delete *it;
                    it = active_windows.erase(it);
                }
                else ++it;
            }
        }
        active = w;
        if (active != nullptr)
            active_windows.push_back(w);
    }
}
