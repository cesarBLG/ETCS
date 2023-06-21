 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "tcp_listener.h"

class BusSocketServer : private PlatformUtil::NoCopy {
private:
	TcpListener listener;
	struct ClientData {
		std::optional<uint32_t> tid;
		uint32_t uid;
		std::unique_ptr<TcpSocket> socket;
		PlatformUtil::Promise<std::string> rx_promise;
		std::string rx_buffer;
	};
	std::vector<ClientData> clients;
	PlatformUtil::Promise<std::unique_ptr<TcpSocket>> accept_promise;
	uint32_t uid;

	void pack_uint32(char* ptr, uint32_t v);
	uint32_t unpack_uint32(const char *ptr);

	void handle_client(ClientData &client);
	void on_client_data(std::string &&data, uint32_t uid);
	void on_new_client(std::unique_ptr<TcpSocket> &&sock);

public:
	BusSocketServer(const std::string &hostname, int port, FdPoller &p);
};

class BusSocketServerManager : private PlatformUtil::NoCopy {
private:
    std::vector<std::unique_ptr<BusSocketServer>> servers;
public:
    BusSocketServerManager(const std::string &load_path, FdPoller &fd);
};
