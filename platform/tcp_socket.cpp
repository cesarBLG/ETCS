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

void TcpSocket::close_socket(int &fd) {
	if (fd == -1)
		return;
	if (&fd == &peer_fd) {
		tx_promise = {};
		rx_promise = {};
		rx_list.fulfill_one("");
	}
#ifdef _WIN32
	closesocket(fd);
#else
	close(fd);
#endif
	fd = -1;
}

void TcpSocket::handle_error(int &fd) {
#ifdef _WIN32
	int wsaerr = WSAGetLastError();
	if (wsaerr != WSAEWOULDBLOCK && wsaerr != WSAEINPROGRESS)
		close_socket(fd);
#else
	if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINPROGRESS)
		close_socket(fd);
#endif
}

void TcpSocket::update() {
	if (peer_fd != -1 && !tx_promise.is_pending() && !tx_buffer.empty()) {
		tx_promise = std::move(poller->on_fd_ready(peer_fd, POLLOUT).then([this](int rev) {
			if (rev & (POLLERR | POLLHUP))
				close_socket(peer_fd);
			if (peer_fd != -1 && (rev & POLLOUT)) {
				ssize_t ret = ::send(peer_fd, tx_buffer.data(), tx_buffer.size(), MSG_NOSIGNAL);
				if (ret < 0)
					handle_error(peer_fd);
				else
					tx_buffer.erase(0, ret);
			}
			update();
		}));
	}

	if (peer_fd != -1 && !rx_promise.is_pending() && rx_list.pending() > 0) {
		rx_promise = std::move(poller->on_fd_ready(peer_fd, POLLIN).then([this](int rev) {
			if (rev & (POLLERR | POLLHUP))
				close_socket(peer_fd);
			if (peer_fd != -1 && (rev & POLLIN)) {
				std::string buf;
				buf.resize(4096);
				ssize_t ret = recv(peer_fd, buf.data(), buf.size(), 0);
				if (ret < 0) {
					handle_error(peer_fd);
				} else if (ret == 0) {
					close_socket(peer_fd);
				} else {
					buf.resize(ret);
					rx_list.fulfill_one(std::move(buf));
				}
			}
			update();
		}));
	}
}

TcpSocket::TcpSocket(TcpSocket &&other) {
	peer_fd = -1;
	*this = std::move(other);
}

TcpSocket& TcpSocket::operator=(TcpSocket &&other) {
	close_socket(peer_fd);
	rx_promise = {};
	tx_promise = {};

	peer_fd = other.peer_fd;
	other.peer_fd = -1;

	rx_list = std::move(other.rx_list);
	tx_buffer = std::move(other.tx_buffer);
	poller = other.poller;

	other.rx_promise = {};
	other.tx_promise = {};
	update();

	return *this;
}

void TcpSocket::connect(const std::string &hostname, int port) {
	close_socket(peer_fd);

	addrinfo hints = {}, *res = nullptr;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &res);

	peer_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	mark_nonblocking(peer_fd);
	::connect(peer_fd, res->ai_addr, res->ai_addrlen);
}

void TcpSocket::send(const std::string &data) {
	tx_buffer += data;
	update();
}

PlatformUtil::Promise<std::string> TcpSocket::receive() {
	PlatformUtil::Promise<std::string> promise = rx_list.create_and_add();
	update();
	return std::move(promise);
}

TcpSocket::TcpSocket(const std::string &hostname, int port, FdPoller &p) : poller(&p) {
	peer_fd = -1;
	connect(hostname, port);
}

TcpSocket::TcpSocket(int fd, FdPoller &p) : poller(&p) {
	peer_fd = fd;
}

TcpSocket::~TcpSocket() {
	close_socket(peer_fd);
}
