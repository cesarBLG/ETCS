 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "platform.h"

std::vector<std::function<void()>> PlatformUtil::DeferredFulfillment::list;

#ifndef EVC
std::unique_ptr<UiPlatform> platform;
#else
std::unique_ptr<BasePlatform> platform;
#endif

#ifdef _WIN32
#include <winsock2.h>
struct WinsockInit {
	WinsockInit() {
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
	}
	~WinsockInit() {
		WSACleanup();
	}
} WinsockInitInstance;
#endif
