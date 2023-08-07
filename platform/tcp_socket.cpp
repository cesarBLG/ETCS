 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "tcp_socket.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <netdb.h>
#include <errno.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef _WIN32
#define socklen_t int
#define ssize_t int
#define MSG_NOSIGNAL 0
#endif

void TcpSocket::mark_nonblocking(int fd) {
	unsigned long one = 1;
#ifdef _WIN32
	ioctlsocket(fd, FIONBIO, &one);
#else
	ioctl(fd, FIONBIO, &one);
#endif
}

void TcpSocket::close_socket() {
	if (peer_fd == -1)
		return;
	tx_promise = {};
	rx_promise = {};
	rx_list.push_data({});
#ifdef _WIN32
	closesocket(peer_fd);
#else
	close(peer_fd);
#endif
	peer_fd = -1;
}

void TcpSocket::handle_error() {
#ifdef _WIN32
	int wsaerr = WSAGetLastError();
	if (wsaerr != WSAEWOULDBLOCK && wsaerr != WSAEINPROGRESS)
		close_socket();
#else
	if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINPROGRESS)
		close_socket();
#endif
}

void TcpSocket::update() {
	if (peer_fd != -1 && !tx_pending && !tx_buffer.empty()) {
		tx_pending = true;
		tx_promise = poller->on_fd_ready(peer_fd, POLLOUT).then([this](int rev) {
			tx_pending = false;
			if (rev & (POLLERR | POLLHUP)) {
				close_socket();
			} else if (peer_fd != -1 && (rev & POLLOUT)) {
				ssize_t ret = ::send(peer_fd, tx_buffer.data(), tx_buffer.size(), MSG_NOSIGNAL);
				if (ret < 0)
					handle_error();
				else
					tx_buffer.erase(0, ret);
			}
			update();
		});
	}

	if (peer_fd != -1 && !rx_pending && rx_list.pending_fulfillers() > 0) {
		rx_pending = true;
		rx_promise = poller->on_fd_ready(peer_fd, POLLIN).then([this](int rev) {
			rx_pending = false;
			if (rev & (POLLERR | POLLHUP)) {
				close_socket();
			} else if (rev & POLLIN) {
				std::string buf;
				buf.resize(4096);
				ssize_t ret = recv(peer_fd, buf.data(), buf.size(), 0);
				if (ret < 0) {
					handle_error();
				} else if (ret == 0) {
					close_socket();
				} else {
					buf.resize(ret);
					rx_list.push_data(std::move(buf));
				}
			}
			update();
		});
	}
}

void TcpSocket::create_and_connect(const std::string_view hostname, int port) {
	addrinfo hints = {}, *res = nullptr;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(std::string(hostname).c_str(), std::to_string(port).c_str(), &hints, &res);

	peer_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	mark_nonblocking(peer_fd);
	connect_promise = poller->on_fd_ready(peer_fd, POLLOUT).then([this](int rev) {
		if (rev & (POLLERR | POLLHUP))
			close_socket();
		else if (rev & POLLOUT)
			connected = true;
	});
	if (::connect(peer_fd, res->ai_addr, res->ai_addrlen) == 0)
		connected = true;
	else
		handle_error();

	freeaddrinfo(res);
}

void TcpSocket::connect(const std::string_view hostname, int port) {
	connected = false;
	if (peer_fd < 0)
		return;
	addrinfo hints = {}, *res = nullptr;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(std::string(hostname).c_str(), std::to_string(port).c_str(), &hints, &res);
	if (res == nullptr) {
		close_socket();
		return;
	}
	mark_nonblocking(peer_fd);
	connect_promise = poller->on_fd_ready(peer_fd, POLLOUT).then([this](int rev) {
		if (rev & (POLLERR | POLLHUP))
			close_socket();
		else if (rev & POLLOUT)
			connected = true;
	});
	if (::connect(peer_fd, res->ai_addr, res->ai_addrlen) == 0)
		connected = true;
	else 
		handle_error();

	freeaddrinfo(res);
}

void TcpSocket::send(const std::string_view data) {
	tx_buffer += data;
	update();
}

PlatformUtil::Promise<std::string> TcpSocket::receive() {
	PlatformUtil::Promise<std::string> promise = rx_list.create_and_add();
	update();
	return std::move(promise);
}

TcpSocket::TcpSocket(const std::string_view hostname, int port, FdPoller &p) : poller(&p), rx_pending(false), tx_pending(false), connected(false) {
	create_and_connect(hostname, port);
}

TcpSocket::TcpSocket(int fd, FdPoller &p) : poller(&p), rx_pending(false), tx_pending(false), connected(false) {
	peer_fd = fd;
}

TcpSocket::~TcpSocket() {
	close_socket();
}

bool TcpSocket::is_connected() {
	return peer_fd >= 0 && connected;
}

bool TcpSocket::is_failed() {
	return peer_fd < 0;
}