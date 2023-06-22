 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"
#include "tcp_socket.h"

class BusSocketImpl : private PlatformUtil::NoCopy {
private:
	FdPoller& poller;
	struct SocketConfig {
		std::string name;
		std::string hostname;
		int port;
	};
	std::vector<SocketConfig> socket_config;

public:
	BusSocketImpl(const std::string &load_path, FdPoller &p);

	std::unique_ptr<BasePlatform::BusSocket> open_bus_socket(const std::string &channel, uint32_t tid);

	class TcpBusSocket final : public BasePlatform::BusSocket {
	private:
		std::unique_ptr<TcpSocket> socket;
		std::string rx_buffer;
		PlatformUtil::Promise<std::string> rx_promise;
		PlatformUtil::Promise<void> retry_promise;
		PlatformUtil::FulfillerList<std::pair<PeerId, std::string>> rx_list;
		PlatformUtil::FulfillerList<PeerId> on_join_list;
		PlatformUtil::FulfillerList<PeerId> on_leave_list;

		void pack_uint32(char* ptr, uint32_t v);
		uint32_t unpack_uint32(const char *ptr);

		void client_hello();
		void data_received(std::string &&data);

		std::string hostname;
		int port;
		uint32_t tid;
		FdPoller& poller;
	public:
		TcpBusSocket(uint32_t tid, const std::string &hostname, int port, FdPoller &poller);

		virtual void broadcast(const std::string &data) override;
		virtual void broadcast(uint32_t tid, const std::string &data) override;
		virtual void send_to(uint32_t uid, const std::string &data) override;
		virtual PlatformUtil::Promise<std::pair<PeerId, std::string>> receive() override;
		virtual PlatformUtil::Promise<PeerId> on_peer_join() override;
		virtual PlatformUtil::Promise<PeerId> on_peer_leave() override;
	};
};
