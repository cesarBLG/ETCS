/*
 * European Train Control System
 * Copyright (C) 2019  Iván Izquierdo
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
#ifndef _INPUT_DATA_H
#define _INPUT_DATA_H
#include <string>
#include <vector>
#include <ctime>
#include "../graphics/button.h"
#include "../graphics/text_button.h"
#include "../graphics/icon_button.h"
#include "../../EVC/Parser/nlohmann/json.hpp"
using namespace std;
using json = nlohmann::json;
class input_data
{
    protected:
    bool selected = false;
    bool accepted = false;
    bool valid = false;
    public:
    bool techrange_invalid=false;
    bool techresol_invalid=false;
    bool techcross_invalid=false;
    bool operatrange_invalid=false;
    bool operatcross_invalid=false;
    string label;
    string data;
    string data_accepted;
    string prev_data;
    string keybd_data;
    text_graphic *data_tex;
    Component* label_comp = nullptr;
    Component* data_comp = nullptr;
    Component* label_echo = nullptr;
    Component* data_echo = nullptr;
    time_t holdcursor;
    void setSelected(bool val);
    void setAccepted(bool val);
    bool isValid() {return valid;}
    bool isAccepted() {return accepted;}
    IconButton more;
    vector<Button*> keys;
    int keypage=0;
    virtual void validate(){if (data!="") valid = true;}
    input_data(string label_text="");
    function<string()> data_get;
    function<void(string)> data_set;
    void setData(string s);
    string getData()
    {
        return data;
    }
    void updateText();
    virtual ~input_data();
};
#endif