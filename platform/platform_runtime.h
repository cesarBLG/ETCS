 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"

extern float platform_size_w;
extern float platform_size_h;
void on_platform_ready();

#ifndef EVC
extern std::unique_ptr<UiPlatform> platform;
#else
extern std::unique_ptr<BasePlatform> platform;
#endif
