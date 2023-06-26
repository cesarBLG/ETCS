 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "console_fd_poller.h"
#include "platform_runtime.h"

#ifndef _WIN32
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifndef _WIN32
#define sys_poll poll
#define socket_type int
#else
#define sys_poll WSAPoll
#define socket_type SOCKET
#endif

void ConsoleFdPoller::poll(int timeout) {
	auto tmp = std::move(fds);
	fds.clear();
	tmp.erase(std::remove_if(tmp.begin(), tmp.end(), [](const auto &entry) { return !entry.second.is_pending(); }), tmp.end());

	std::vector<pollfd> pfd;
	pfd.reserve(tmp.size());
	for (const auto &entry : tmp)
		pfd.push_back({ (socket_type)entry.first.first, entry.first.second, 0 });

	::sys_poll(pfd.data(), pfd.size(), timeout);

	for (int i = 0; i < pfd.size(); i++) {
		if (pfd[i].revents & POLLNVAL)
			platform->debug_print("POLLNVAL!");
		else if ((pfd[i].events & pfd[i].revents) != 0 || (pfd[i].revents & (POLLERR | POLLHUP)) != 0)
			tmp[i].second.fulfill(pfd[i].revents, false);
		else
			fds.push_back(std::move(tmp[i]));
	}
}

PlatformUtil::Promise<short> ConsoleFdPoller::on_fd_ready(int fd, short ev) {
	auto pair = PlatformUtil::PromiseFactory::create<short>();
	fds.push_back(std::make_pair(std::make_pair(fd, ev), std::move(pair.second)));
	return std::move(pair.first);
}

bool ConsoleFdPoller::is_empty() {
	return fds.empty();
}
