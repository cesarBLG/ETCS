 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "tcp_listener.h"
#include <fstream>
#include <sstream>
#include <cstring>

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

void TcpListener::close_socket(int &fd) {
	if (fd == -1)
		return;
	if (&fd == &listen_fd)
		promise = {};
#ifdef _WIN32
	closesocket(fd);
#else
	close(fd);
#endif
	fd = -1;
}

void TcpListener::handle_error(int &fd) {
#ifdef _WIN32
	int wsaerr = WSAGetLastError();
	if (wsaerr != WSAEWOULDBLOCK && wsaerr != WSAEINPROGRESS)
		close_socket(fd);
#else
	if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINPROGRESS)
		close_socket(fd);
#endif
}

void TcpListener::fd_ready(int rev) {
	if (rev & (POLLERR | POLLHUP)) {
		close_socket(listen_fd);
		return;
	}
	if (rev & POLLIN) {
		sockaddr_storage addr;
		size_t addr_size = sizeof(addr);

		int ret = ::accept(listen_fd, (struct sockaddr*)&addr, (socklen_t*)&addr_size);
		if (ret < 0) {
			handle_error(listen_fd);
		} else {
			int peer_fd = ret;
			mark_nonblocking(peer_fd);
			list.fulfill_one(TcpSocket(peer_fd, poller));
		}
	}
	if (listen_fd != -1)
		promise = std::move(poller.on_fd_ready(listen_fd, POLLIN).then(std::bind(&TcpListener::fd_ready, this, std::placeholders::_1)));
}

TcpListener::TcpListener(const std::string &hostname, int port, FdPoller &p) : poller(p) {
#ifdef _WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

	listen_fd = -1;
	addrinfo hints, *res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &res);

	listen_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	mark_nonblocking(listen_fd);
	int one = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	bind(listen_fd, res->ai_addr, res->ai_addrlen);
	listen(listen_fd, 10);

	promise = std::move(poller.on_fd_ready(listen_fd, POLLIN).then(std::bind(&TcpListener::fd_ready, this, std::placeholders::_1)));
}

TcpListener::~TcpListener() {
	close_socket(listen_fd);
#ifdef _WIN32
	WSACleanup();
#endif
}

PlatformUtil::Promise<TcpSocket> TcpListener::accept() {
	return list.create_and_add();
}
