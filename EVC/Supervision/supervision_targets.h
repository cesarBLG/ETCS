/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "targets.h"
extern std::shared_ptr<target> MRDT;
extern std::shared_ptr<target> RSMtarget;
extern distance d_startRSM;
extern std::shared_ptr<target> indication_target;