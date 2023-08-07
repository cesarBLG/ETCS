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
std::map<int,SoftKey> softF;
std::map<int,SoftKey> softH;
ExternalAckButton externalAckButton;
void setupSoftKeys()
{
    for (int i=1; i<11; i++) {
        softF[i] = SoftKey(-32+64*i, 455);
    }
    softH[1] = SoftKey(620, 14);
    for (int i=2; i<7; i++) {
        softH[i] = SoftKey(620, 60+64*(i-2));
    }
    softH[7] = SoftKey(620, 413);
}