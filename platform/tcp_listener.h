 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"
#include "tcp_socket.h"

class TcpListener : private PlatformUtil::NoCopy
{
private:
	int listen_fd;
	PlatformUtil::FulfillerList<TcpSocket> list;
	PlatformUtil::Promise<short> promise;
	FdPoller& poller;

	void mark_nonblocking(int fd);
	void close_socket(int &fd);
	void handle_error(int &fd);

	void fd_ready(int rev);

public:
	TcpListener(const std::string &hostname, int port, FdPoller &p);
	~TcpListener();
	PlatformUtil::Promise<TcpSocket> accept();
};
