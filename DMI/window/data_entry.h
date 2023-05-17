/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _DATA_ENTRY_H
#define _DATA_ENTRY_H
#include "subwindow.h"
#include "input_data.h"
#include <nlohmann/json.hpp>
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
    std::map<int, input_data*> inputs;
    Button* buttons[12];
    virtual void setLayout() override;
    virtual void sendInformation();
    void create();
    public:
    input_window(std::string name, int nfields, bool fullscreen);
    void buildFrom(json &j);
    void fieldCheckResult(json &j);
    void crossResult(json &j);
    void inputChanged(input_data *input);
    virtual ~input_window();
};
#endif