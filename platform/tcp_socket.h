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
	PlatformUtil::FulfillerBufferedQueue<std::string> rx_list;
	PlatformUtil::FulfillerList<void> shut_list;
	std::string tx_buffer;
	PlatformUtil::Promise<short> rx_promise;
	PlatformUtil::Promise<short> tx_promise;
	bool rx_pending, tx_pending, shut_rd, shut_wr, shut_wrq;
	FdPoller& poller;

	void mark_nonblocking(int fd);
	void close_socket();
	void handle_error();
	void update();
	void connect(const std::string_view hostname, int port);
public:
	TcpSocket(const std::string_view hostname, int port, FdPoller &p);
	TcpSocket(int fd, FdPoller &p);
	~TcpSocket();
	void send(const std::string_view data);
	void shutdown();
	PlatformUtil::Promise<void> on_shutdown();
	PlatformUtil::Promise<std::string> receive();
};
