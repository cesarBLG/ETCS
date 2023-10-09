/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <list>
#include "../graphics/color.h"
#include "../graphics/button.h"
#include "../graphics/component.h"
#include "../graphics/drawing.h"
#include "../messages/messages.h"
#include "../window/window.h"
#include "../../EVC/Packets/STM/43.h"
#include "../../EVC/Packets/STM/message.h"
#include "../sound/sound.h"
#include <nlohmann/json.hpp>
#include "../../EVC/STM/stm_state.h"
using json = nlohmann::json;
struct stm_monitoring_data
{
    Color needle_color;
    int Vperm_display;
    Color Vperm_color;
    int Vtarget_display;
    Color Vtarget_color;
    int Vsbi_display;
    Color Vsbi_color;
    int Vrelease_display;
    Color Vrelease_color;
    int Dtarget_display;
    stm_monitoring_data() = default;
    stm_monitoring_data(const STMSupervisionInformation &info)
    {
        needle_color = Color::from_etcs(info.M_COLOUR_SP.get_value());
        Vperm_display = info.Q_DISPLAY_PS.rawdata;
        Vperm_color = Color::from_etcs(info.M_COLOUR_PS.get_value());
        Vtarget_display = info.Q_DISPLAY_TS.rawdata;
        Vtarget_color = Color::from_etcs(info.M_COLOUR_TS.get_value());
        Vrelease_display = info.Q_DISPLAY_RS.rawdata;
        Vrelease_color = Color::from_etcs(info.M_COLOUR_RS.get_value());
        Vsbi_display = info.Q_DISPLAY_IS.rawdata;
        Vsbi_color = Color::from_etcs(info.M_COLOUR_IS.get_value());
        Dtarget_display = info.Q_DISPLAY_TD.rawdata;
    }
};
void construct_main(window *w, bool customized);
struct customized_dmi
{
    int nid_stm;
    struct indicator
    {
        int font_size;
        int align;
        indicator() = default;
        indicator(const json &j)
        {
            font_size = j["font_size"].get<int>();
            std::string h = j["halign"].get<std::string>();
            align = h == "center" ? CENTER : (h == "left" ? LEFT : RIGHT);
            std::string v = j["valign"].get<std::string>();
            align |= h == "center" ? CENTER : (h == "up" ? UP : DOWN);
        }
    };
    struct moved_area
    {
        int x;
        int y;
        std::string soft_key;
    };
    std::map<int, indicator> indicators;
    std::map<int, indicator> buttons;
    std::map<int, std::vector<int>> positions;
    std::map<int, std::vector<int>> button_positions;
    struct icon
    {
        std::string file;
        bool text_also;
    };
    std::map<int, icon> icons;
    bool etcs_supervision;
    int etcs_dial_range;
    double slow_flash_freq;
    double fast_flash_freq;
    int flash_style;
    std::map<int, std::unique_ptr<StmSound>> sounds;
    std::map<std::string,moved_area> moved_areas;
    std::map<std::string, std::pair<int,int>> softkeys_coordinates;
    customized_dmi(json &j)
    {
        nid_stm = j["nid_stm"].get<int>();
        for (auto &inds : j["indicators"])
        {
            indicators[inds["id"].get<int>()] = indicator(inds);
        }
        for (auto &inds : j["buttons"])
        {
            buttons[inds["id"].get<int>()] = indicator(inds);
        }
        for (auto &pos : j[softkeys ? "positions_softkeys" : "positions"])
        {
            positions[pos["id"].get<int>()] = {pos["x"].get<int>(),pos["y"].get<int>(),pos["width"].get<int>(),pos["height"].get<int>()};
        }
        for (auto &pos : j[softkeys ? "button_positions_softkeys" : "button_positions"])
        {
            std::vector<int> coords = {pos["x"].get<int>(),pos["y"].get<int>(),pos["width"].get<int>(),pos["height"].get<int>()};
            button_positions[pos["id"].get<int>()] = coords;
            if (pos.contains("softkey"))
                softkeys_coordinates[pos["softkey"]] = {coords[0]+1,coords[1]+1};
        }
        for (auto &ics : j["icons"])
        {
            icon ic;
            ic.file = ics["file"].get<std::string>();
            ic.text_also = ics["text"].get<bool>();
            icons[ics["id"].get<int>()] = ic;
        }
        etcs_supervision = j["etcs_supervision"].get<bool>();
        etcs_dial_range = j["etcs_dial_range"].get<int>();
        slow_flash_freq = j["flash_slow"].get<double>();
        fast_flash_freq = j["flash_fast"].get<double>();
        flash_style = j["flash_style"].get<std::string>() == "area" ? 1 : 0;
        for (auto &snds : j["sounds"])
        {
            sounds[snds["id"].get<int>()] = loadStmSound(snds["file"].get<std::string>());
        }
        if (j.contains("moved_areas"))
        {
            for (auto &area : j["moved_areas"])
            {
                moved_area a;
                a.x = area["x"];
                a.y = area["y"];
                moved_areas[area["area"]] = a;
                if (area.contains("softkey"))
                    softkeys_coordinates[area["softkey"]] = {a.x + 1, a.y + 1};
            }
        }
    }
};
class ntc_window : public window
{
    int nid_stm;
    std::map<int, Component*> indicators;
    std::map<int, std::shared_ptr<UiPlatform::Image>> icons;
    public:
    stm_state state;
    int64_t last_time;
    std::map<int, Message> messages;
    std::list<std::pair<int, std::unique_ptr<StmSound>>> generated_sounds;
    customized_dmi *customized;
    stm_monitoring_data monitoring_data;
    Color get_color(int col, bool bg)
    {
        switch(col)
        {
            case 0:
                return bg ? DarkBlue : Black;
            case 1:
                return White;
            case 2:
                return Red;
            case 3:
                return Blue;
            case 4:
                return Green;
            case 5:
                return Yellow;
            case 6:
                return LightRed;
            case 7:
                return LightGreen;
        }
        return White;
    }
    public:
    ntc_window(int nid_stm);
    void display_indicator(int id, int position, int icon, std::string text, int properties, bool isButton);
    void display_text(int id, bool ack, std::string text, int properties);
    ~ntc_window()
    {
        for (auto &it : indicators)
        {
            delete it.second;
        }
        for (auto &it : messages)
        {
            revokeMessage(it.second.Id);
        }
        if (customized != nullptr) delete customized;
    }
};
extern ntc_window *active_ntc_window;
void initialize_ntc_windows();
void update_ntc_windows();
void parse_stm_message(stm_message &message);