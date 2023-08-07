 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"

class LibcTimeImpl {
public:
	int64_t get_timer();
	int64_t get_timestamp();
	BasePlatform::DateTime get_local_time();
};
