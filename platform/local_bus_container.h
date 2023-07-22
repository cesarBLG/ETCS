 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"
#include "local_bus_socket.h"
#include <map>

class LocalBusContainer : private PlatformUtil::NoCopy {
private:
	std::map<std::string, std::shared_ptr<LocalBusRouter>, std::less<>> busses;

public:
	void add_bus(const std::string_view name, std::shared_ptr<LocalBusRouter> bus);
	void remove_bus(const std::string_view name);

	std::unique_ptr<BasePlatform::BusSocket> open_bus_socket(const std::string_view channel, uint32_t tid);
};
