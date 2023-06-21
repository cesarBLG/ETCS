 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "tcp_listener.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef _WIN32
#define socklen_t int
#define ssize_t int
#define MSG_NOSIGNAL 0
#endif

void TcpListener::mark_nonblocking(int fd) {
	unsigned long one = 1;
#ifdef _WIN32
	ioctlsocket(fd, FIONBIO, &one);
#else
	ioctl(fd, FIONBIO, &one);
#endif
}

void TcpListener::close_socket() {
	if (listen_fd == -1)
		return;
	promise = {};
#ifdef _WIN32
	closesocket(listen_fd);
#else
	close(listen_fd);
#endif
	listen_fd = -1;
}

void TcpListener::handle_error() {
#ifdef _WIN32
	int wsaerr = WSAGetLastError();
	if (wsaerr != WSAEWOULDBLOCK && wsaerr != WSAEINPROGRESS)
		close_socket();
#else
	if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINPROGRESS)
		close_socket();
#endif
}

void TcpListener::fd_ready(int rev) {
	if (rev & (POLLERR | POLLHUP)) {
		close_socket();
		return;
	}
	if (rev & POLLIN) {
		sockaddr_storage addr;
		size_t addr_size = sizeof(addr);

		int ret = ::accept(listen_fd, (struct sockaddr*)&addr, (socklen_t*)&addr_size);
		if (ret < 0) {
			handle_error();
		} else {
			int peer_fd = ret;
			mark_nonblocking(peer_fd);
			list.fulfill_one(std::make_unique<TcpSocket>(peer_fd, poller));
		}
	}
	if (listen_fd != -1)
		promise = std::move(poller.on_fd_ready(listen_fd, POLLIN).then(std::bind(&TcpListener::fd_ready, this, std::placeholders::_1)));
}

TcpListener::TcpListener(const std::string &hostname, int port, FdPoller &p) : poller(p) {
	listen_fd = -1;
	addrinfo hints = {}, *res = nullptr;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &res);

	listen_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	mark_nonblocking(listen_fd);
	int one = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(one));
	bind(listen_fd, res->ai_addr, res->ai_addrlen);
	listen(listen_fd, 10);

	freeaddrinfo(res);

	promise = std::move(poller.on_fd_ready(listen_fd, POLLIN).then(std::bind(&TcpListener::fd_ready, this, std::placeholders::_1)));
}

TcpListener::~TcpListener() {
	close_socket();
}

PlatformUtil::Promise<std::unique_ptr<TcpSocket>> TcpListener::accept() {
	return list.create_and_add();
}
