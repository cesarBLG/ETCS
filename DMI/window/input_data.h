/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _INPUT_DATA_H
#define _INPUT_DATA_H
#include <string>
#include <vector>
#include <ctime>
#include "../graphics/button.h"
#include "../graphics/text_button.h"
#include "../graphics/icon_button.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
class input_window;
class input_data
{
    protected:
    bool selected = false;
    bool accepted = false;
    bool valid = false;
    public:
    bool character_separation = false;
    bool techrange_invalid=false;
    bool techresol_invalid=false;
    bool techcross_invalid=false;
    bool operatrange_invalid=false;
    bool operatcross_invalid=false;
    bool show_echo;
    std::string label;
    std::string data;
    std::string data_accepted;
    std::string prev_data;
    std::string keybd_data;
    text_graphic *data_tex;
    Component* label_comp = nullptr;
    Button* data_comp = nullptr;
    Component* label_echo = nullptr;
    Component* data_echo = nullptr;
    input_window* window = nullptr;
    time_t holdcursor;
    void setSelected(bool val);
    void setAccepted(bool val);
    bool isAccepted() {return accepted;}
    IconButton more;
    std::vector<Button*> keys;
    int keypage=0;
    input_data(std::string label_text="", bool echo=true);
    std::function<std::string()> data_get;
    std::function<void(std::string)> data_set;
    void setData(std::string s);
    std::string getData()
    {
        return data;
    }
    std::string getFormattedData(std::string data)
    {
        if (!character_separation) return data;
        std::string format = "";
        for (int i=0; i<data.size(); i++) {
            if (i>0 && i%8 == 0) format += "\n";
            else if (i>0 && i%4 == 0 && i != data.size() - 1) format += " ";
            format += data[i];
        }
        return format;
    }
    void updateText();
    virtual ~input_data();
};
#endif