/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _KEYBOARDS_H
#define _KEYBOARDS_H
#include "keyboard.h"
#include "input_data.h"
keyboard getNumericKeyboard(input_data *input);
keyboard getAlphaNumericKeyboard(input_data *input);
keyboard getSingleChoiceKeyboard(std::vector<std::string> posibilities, input_data *input);
keyboard getYesNoKeyboard(input_data *input);
keyboard getKeyboard(const json &j, input_data *input);
#endif