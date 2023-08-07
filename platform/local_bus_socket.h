 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"
#include <unordered_map>
#include <unordered_set>

class LocalBusRouter;

class LocalBusSocket final : public BasePlatform::BusSocket {
	friend class LocalBusRouter;
	struct CreateTicket { };

	std::shared_ptr<LocalBusRouter> router;
	PeerId id;

public:
	LocalBusSocket(std::shared_ptr<LocalBusRouter> r, PeerId id, CreateTicket t);
	~LocalBusSocket() override;

	void broadcast(const std::string_view msg) override;
	void broadcast(uint32_t tid, const std::string_view msg) override;
	void send_to(uint32_t uid, const std::string_view msg) override;
	PlatformUtil::Promise<LocalBusSocket::ReceiveResult> receive() override;
};

class LocalBusRouter : public std::enable_shared_from_this<LocalBusRouter>, private PlatformUtil::NoCopy {
private:
	friend class LocalBusSocket;
	struct CreateTicket { };
	uint32_t uid_counter;

	std::unordered_map<LocalBusSocket::PeerId, PlatformUtil::FulfillerBufferedQueue<LocalBusSocket::ReceiveResult>> receive_list;
	std::unordered_set<LocalBusSocket::PeerId> peers;

	void broadcast(LocalBusSocket::PeerId id, const std::string_view msg);
	void broadcast(LocalBusSocket::PeerId id, uint32_t tid, const std::string_view msg);
	void send_to(LocalBusSocket::PeerId id, uint32_t uid, const std::string_view msg);
	PlatformUtil::Promise<LocalBusSocket::ReceiveResult> receive(LocalBusSocket::PeerId id);

	void remove_peer(LocalBusSocket::PeerId id);

public:
	LocalBusRouter(CreateTicket t);

	static std::shared_ptr<LocalBusRouter> create_router();
	std::unique_ptr<LocalBusSocket> create_peer(uint32_t tid);
};
