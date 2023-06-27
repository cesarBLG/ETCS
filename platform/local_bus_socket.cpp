 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "local_bus_socket.h"

using PlatformUtil::Promise;

LocalBusRouter::LocalBusRouter(CreateTicket t) : uid_counter(0) {
}

std::shared_ptr<LocalBusRouter> LocalBusRouter::create_router() {
	return std::make_shared<LocalBusRouter>(CreateTicket());
}

void LocalBusRouter::broadcast(LocalBusSocket::PeerId id, const std::string_view msg) {
	std::string data(msg);
	for (auto &entry : receive_list)
		if (entry.first.uid != id.uid)
			entry.second.push_data(LocalBusSocket::Message { id, data });
}

void LocalBusRouter::broadcast(LocalBusSocket::PeerId id, uint32_t tid, const std::string_view msg) {
	std::string data(msg);
	for (auto &entry : receive_list)
		if (entry.first.uid != id.uid && entry.first.tid == tid)
			entry.second.push_data(LocalBusSocket::Message { id, data });
}

void LocalBusRouter::send_to(LocalBusSocket::PeerId id, uint32_t uid, const std::string_view msg) {
	std::string data(msg);
	for (auto &entry : receive_list)
		if (entry.first.uid != id.uid && entry.first.uid == uid)
			entry.second.push_data(LocalBusSocket::Message { id, data });
}

Promise<LocalBusSocket::ReceiveResult> LocalBusRouter::receive(LocalBusSocket::PeerId id) {
	return receive_list.find(id)->second.create_and_add();
}

std::unique_ptr<LocalBusSocket> LocalBusRouter::create_peer(uint32_t tid) {
	LocalBusSocket::PeerId id { tid, ++uid_counter };
	for (auto &entry : receive_list)
		entry.second.push_data(LocalBusSocket::JoinNotification{ id });

	auto it = receive_list.insert(std::make_pair(id, PlatformUtil::FulfillerBufferedQueue<LocalBusSocket::ReceiveResult>())).first;
	for (const auto &p : peers)
		it->second.push_data(LocalBusSocket::JoinNotification{ p });
	peers.insert(id);
	return std::make_unique<LocalBusSocket>(shared_from_this(), id, LocalBusSocket::CreateTicket());
}

void LocalBusRouter::remove_peer(LocalBusSocket::PeerId id) {
	receive_list.erase(id);
	peers.erase(id);
	for (auto &entry : receive_list)
		entry.second.push_data(LocalBusSocket::LeaveNotification{ id });
}

LocalBusSocket::LocalBusSocket(std::shared_ptr<LocalBusRouter> r, PeerId i, CreateTicket t) : router(r), id(i) {
}

LocalBusSocket::~LocalBusSocket() {
	router->remove_peer(id);
}

void LocalBusSocket::broadcast(const std::string_view msg) {
	router->broadcast(id, msg);
}

void LocalBusSocket::broadcast(uint32_t tid, const std::string_view msg) {
	router->broadcast(id, tid, msg);
}

void LocalBusSocket::send_to(uint32_t uid, const std::string_view msg) {
	router->send_to(id, uid, msg);
}

Promise<LocalBusSocket::ReceiveResult> LocalBusSocket::receive() {
	return router->receive(id);
}
