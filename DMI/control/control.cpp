/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "control.h"
#include "../graphics/display.h"
#include "../graphics/drawing.h"
#include "../monitor.h"
#include "../window/window.h"
#include "../window/data_view.h"
#include "../window/level_window.h"
#include "../window/train_data.h"
#include "../window/fixed_train_data.h"
#include "../window/rbc_data.h"
#include "../window/driver_id.h"
#include "../window/running_number.h"
#include "../window/sr_data.h"
#include "../window/menu_main.h"
#include "../window/menu_radio.h"
#include "../window/menu_override.h"
#include "../window/menu_spec.h"
#include "../window/menu_settings.h"
#include "../window/set_vbc.h"
#include "../window/track_ahead_free.h"
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
subwindow *active = nullptr;
std::string active_name;
mutex draw_mtx;
#include <iostream>
void startWindows()
{
    main_window.construct();
    navigation_bar.construct();
    planning_area.construct();
    taf_window.construct();
    active_windows.insert(&main_window);
    active_windows.insert(&navigation_bar);
    active_windows.insert(&planning_area);
    active_windows.insert(&taf_window);
    taf_window.active = false;
    active_name = "default";
}
void setWindow(json &j)
{
    subwindow *w = nullptr;
    std::string name = j["active"].get<std::string>();
    if (name == "default") {
        extern bool showSpeeds;
        navigation_bar.active = main_window.active = true;
        planning_area.active = !display_taf && (mode == Mode::FS || (mode == Mode::OS && showSpeeds));
        taf_window.active = display_taf;
    } else {
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
        } else if (name == "driver_window") {
            driver_window *d;
            if (same) d = (driver_window*)active;
            else d = new driver_window(j["driver_id"].get<std::string>(), j["show_trn"].get<bool>());
            w = d;
        } else if (name == "trn_window") {
            trn_window *d;
            if (same) d = (trn_window*)active;
            else d = new trn_window(j["trn"].get<int>());
            w = d;
        } else if (name == "level_window") {
            level_window *l;
            if (same) l = (level_window*)active;
            else l = new level_window(j["level"].get<std::string>(), j["Levels"].get<std::vector<std::string>>());
            w = l;
        } else if (name == "level_validation_window") {
            level_validation_window *l;
            if (same) l = (level_validation_window*)active;
            else l = new level_validation_window(j["level"].get<std::string>());
            w = l;
        } else if (name == "train_data_window") {
            train_data_window *t;
            if (same) t = (train_data_window*)active;
            else t = new train_data_window();
            w = t;
        } else if (name == "fixed_train_data_window") {
            fixed_train_data_window *t;
            if (same) t = (fixed_train_data_window*)active;
            else t = new fixed_train_data_window(j["train_data"].get<std::string>());
            w = t;
        } else if (name == "fixed_train_data_validation_window") {
            fixed_train_data_validation_window *t;
            if (same) t = (fixed_train_data_validation_window*)active;
            else t = new fixed_train_data_validation_window(j["train_data"].get<std::string>());
            w = t;
        } else if (name == "rbc_data_window") {
            rbc_data_window *t;
            if (same) t = (rbc_data_window*)active;
            else t = new rbc_data_window(j["RBC id"].get<std::uint32_t>(), j["RBC phone number"].get<std::uint64_t>());
            w = t;
        } else if (name == "set_vbc_window") {
            set_vbc_window *t;
            if (same) t = (set_vbc_window*)active;
            else t = new set_vbc_window();
            w = t;
        } else if (name == "remove_vbc_window") {
            remove_vbc_window *t;
            if (same) t = (remove_vbc_window*)active;
            else t = new remove_vbc_window();
            w = t;
        } else if (name == "sr_data_window") {
            sr_data_window *t;
            if (same) t = (sr_data_window*)active;
            else t = new sr_data_window();
            w = t;
        } else if (name == "data_view_window") {
            data_view_window *t;
            if (same)
            {
                t = (data_view_window*)active;
            }
            else
            {
                std::vector<std::pair<std::string,std::string>> data;
                json &fields = j["Fields"];
                for (auto it = fields.begin(); it!=fields.end(); ++it)
                {
                    data.push_back({it.key(), it.value().get<std::string>()});
                }
                t = new data_view_window(data);
            }
            w = t;
        } else {
            if (same) w = active;
            else
            {
                json& def = j["WindowDefinition"];
                std::string type = def["WindowType"].get<std::string>();
                if (type == "DataEntry")
                {
                    w = new input_window(def["WindowTitle"].get<std::string>(), def["Inputs"].size(), false);
                    ((input_window*)w)->build_from(def);
                }
                
                /*else if (type == "Menu") w = new menu(j["WindowDefinitionW"]);*/
            }
        }
        if (w != nullptr)
        {
            w->exit_button.enabled = !j.contains("enabled") || !j["enabled"].contains("Exit") || j["enabled"]["Exit"].get<bool>();
        }
        navigation_bar.active = planning_area.active = false;
        main_window.active = w == nullptr || !w->fullscreen;
    }
    active_name = name;
    if (active != w) {
        if (active != nullptr) {
            old_windows.insert(active);
            active_windows.erase(active);
        }
        active = w;
        if (active != nullptr)
            active_windows.insert(w);
        repaint();
    }
}