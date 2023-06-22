 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"

class FdPoller {
public:
	virtual PlatformUtil::Promise<short> on_fd_ready(int fd, short ev) = 0;
};

class TcpSocket : private PlatformUtil::NoCopy
{
private:
	int peer_fd;
	PlatformUtil::FulfillerList<std::string> rx_list;
	std::string tx_buffer;
	PlatformUtil::Promise<short> rx_promise;
	PlatformUtil::Promise<short> tx_promise;
	bool rx_pending, tx_pending;
	FdPoller* poller;

	void mark_nonblocking(int fd);
	void close_socket();
	void handle_error();
	void update();
	void connect(const std::string &hostname, int port);
public:
	TcpSocket(const std::string &hostname, int port, FdPoller &p);
	TcpSocket(int fd, FdPoller &p);
	~TcpSocket();
	void send(const std::string &data);
	PlatformUtil::Promise<std::string> receive();
};
