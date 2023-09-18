/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "softkey.h"
#include <map>
#include "../STM/stm_objects.h"
#include "../graphics/display.h"
std::map<int,SoftKey> softF;
std::map<int,SoftKey> softH;
ExternalAckButton externalAckButton;
SoftKey::SoftKey(float x, float y, std::string name) : x(x), y(y), name(name) {}
void SoftKey::setPressed(bool pressed)
{
    if (active_ntc_window != nullptr && active_windows.back() == active_ntc_window && active_ntc_window->customized != nullptr && (name[0] == 'F' || name == "H2" || name == "H3" || name == "H4"))
    {
        auto *cust = active_ntc_window->customized;
        if (cust->softkeys_coordinates.find(name) != cust->softkeys_coordinates.end())
        {
            auto &coords = cust->softkeys_coordinates[name];
            input_received({pressed ? UiPlatform::InputEvent::Action::Press : UiPlatform::InputEvent::Action::Release, coords.first, coords.second});
            return;
        }
        if (cust->moved_areas.find(name) != cust->moved_areas.end())
            return;
        if (name != "F1" && name != "F2" && name != "F3" && name != "F4" && name != "F5")
            return;
    }
    input_received({pressed ? UiPlatform::InputEvent::Action::Press : UiPlatform::InputEvent::Action::Release, x, y});
}
void setupSoftKeys()
{
    for (int i=1; i<11; i++) {
        softF[i] = SoftKey(-32+64*i, 455, "F"+std::to_string(i));
    }
    softH[1] = SoftKey(620, 14, "H1");
    for (int i=2; i<7; i++) {
        softH[i] = SoftKey(620, 60+64*(i-2), "H"+std::to_string(i));
    }
    softH[7] = SoftKey(620, 389, "H7");
}