 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "libc_time_impl.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <chrono>

int64_t LibcTimeImpl::get_timer() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

int64_t LibcTimeImpl::get_timestamp() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

BasePlatform::DateTime LibcTimeImpl::get_local_time() {
	time_t now = time(nullptr);
	tm *datetime = localtime(&now);
	return BasePlatform::DateTime { 
		datetime->tm_year + 1900, datetime->tm_mon, datetime->tm_mday - 1,
		datetime->tm_hour, datetime->tm_min, datetime->tm_sec
	};
}
