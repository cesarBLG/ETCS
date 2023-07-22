 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "local_bus_container.h"

void LocalBusContainer::add_bus(const std::string_view channel, std::shared_ptr<LocalBusRouter> bus) {
	busses.insert_or_assign(std::string(channel), bus);
}

void LocalBusContainer::remove_bus(const std::string_view channel) {
	auto it = busses.find(channel);
	if (it != busses.end())
		busses.erase(it);
}

std::unique_ptr<BasePlatform::BusSocket> LocalBusContainer::open_bus_socket(const std::string_view channel, uint32_t tid) {
	auto it = busses.find(channel);
	if (it == busses.end())
		return nullptr;
	return it->second->create_peer(tid);
}
