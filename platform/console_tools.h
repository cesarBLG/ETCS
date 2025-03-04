 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "platform_runtime.h"
#ifdef __ANDROID__
extern std::string android_external_storage_dir;
#endif
std::string get_files_dir(FileType type);
void setup_crash_handler();