 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "platform.h"

#ifndef EVC
std::unique_ptr<UiPlatform> platform;
#else
std::unique_ptr<BasePlatform> platform;
#endif
