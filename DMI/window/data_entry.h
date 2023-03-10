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
 */#ifndef _DATA_ENTRY_H
#define _DATA_ENTRY_H
#include "subwindow.h"
#include "input_data.h"
#include "../../EVC/Parser/nlohmann/json.hpp"
#include <map>
using json = nlohmann::json;
class input_window : public subwindow
{
    TextButton button_yes;
    Button* empty_button[12];
    int cursor=0;
    int nfields;
    protected:
    Component confirmation_label;
    map<int, input_data*> inputs;
    Button* buttons[12];
    virtual void setLayout() override;
    void sendInformation();
    void create();
    public:
    input_window(string name, int nfields, bool fullscreen);
    void buildFrom(json &j);
    void fieldCheckResult(json &j);
    void crossResult(json &j);
    void inputChanged(input_data *input);
    virtual ~input_window();
};
#endif