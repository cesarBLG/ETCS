#pragma once
#include <list>
#include "../graphics/color.h"
#include "../graphics/button.h"
#include "../graphics/component.h"
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
        needle_color = info.M_COLOUR_SP.get_value();
        Vperm_display = info.Q_DISPLAY_PS.rawdata;
        Vperm_color = info.M_COLOUR_PS.get_value();
        Vtarget_display = info.Q_DISPLAY_TS.rawdata;
        Vtarget_color = info.M_COLOUR_TS.get_value();
        Vrelease_display = info.Q_DISPLAY_RS.rawdata;
        Vrelease_color = info.M_COLOUR_RS.get_value();
        Vsbi_display = info.Q_DISPLAY_IS.rawdata;
        Vsbi_color = info.M_COLOUR_IS.get_value();
        Dtarget_display = info.Q_DISPLAY_TD.rawdata;
    }
};
void construct_main(window *w);
struct customized_dmi
{
    int nid_stm;
    struct indicator
    {
        int font_size;
        int align;
    };
    std::map<int, indicator> indicators;
    std::map<int, std::vector<int>> positions;
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
    std::map<int, sdlsounddata*> sounds;
    customized_dmi(json &j)
    {
        nid_stm = j["nid_stm"].get<int>();
        for (auto &inds : j["indicators"])
        {
            indicator ind;
            ind.font_size = inds["font_size"].get<int>();
            std::string h = inds["halign"].get<std::string>();
            ind.align = h == "center" ? CENTER : (h == "left" ? LEFT : RIGHT);
            std::string v = inds["valign"].get<std::string>();
            ind.align |= h == "center" ? CENTER : (h == "up" ? UP : DOWN);
            indicators[inds["id"].get<int>()] = ind;
        }
        for (auto &pos : j["positions"])
        {
            positions[pos["id"].get<int>()] = {pos["x"].get<int>(),pos["y"].get<int>(),pos["width"].get<int>(),pos["height"].get<int>()};
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
            sounds[snds["id"].get<int>()] = loadSound(snds["file"].get<std::string>());
        }
    }
    ~customized_dmi()
    {
        for (auto &kvp : sounds)
        {
            stopSound(kvp.second);
            delete kvp.second;
        }
    }
};
class ntc_window : public window
{
    int nid_stm;
    std::map<int, Component*> indicators;
    //std::map<int, image_graphic*> icons;
    public:
    stm_state state;
    int64_t last_time;
    std::map<int, Message> messages;
    std::list<std::pair<int, sdlsounddata*>> generated_sounds;
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
        for (auto &snd : generated_sounds)
        {
            stopSound(snd.second);
            delete snd.second;
        }
        if (customized != nullptr) delete customized;
    }
};
extern ntc_window *active_ntc_window;
void initialize_ntc_windows();
void update_ntc_windows();
void parse_stm_message(const stm_message &message);