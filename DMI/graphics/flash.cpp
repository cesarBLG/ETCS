/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "flash.h"
#include "platform_runtime.h"

int flash_state = 0;

std::function<void()> flash_func;
void setupFlash() {
    flash_func = [](){
        flash_state = (flash_state + 1) % 4;
        platform->delay(250).then(flash_func).detach();
    };
    flash_func();
}
