 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "console_platform.h"

#include <signal.h>
#include <sys/poll.h>

static std::atomic<bool>* quit_request_ptr;

static void sigterm_handler(int sig) {
	*quit_request_ptr = true;
}

ConsolePlatform::ConsolePlatform(const std::string &path) :
	load_path(path),
	bus_socket_impl(load_path, poller),
	fstream_file_impl(load_path)
#ifdef EVC
	,
	bus_server_manager(load_path, poller),
	bus_bridge_manager(load_path, poller, bus_socket_impl)
#endif
	{
	running = true;
	quit_request = false;
#ifdef __unix__
	quit_request_ptr = &quit_request;
	signal(SIGTERM, &sigterm_handler);
	signal(SIGINT, &sigterm_handler);
#endif
}

ConsolePlatform::~ConsolePlatform() {
#ifdef __unix__
	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);
#endif
}

int64_t ConsolePlatform::get_timer() {
	return libc_time_impl.get_timer();
}

int64_t ConsolePlatform::get_timestamp() {
	return libc_time_impl.get_timestamp();
}

ConsolePlatform::DateTime ConsolePlatform::get_local_time() {
	return libc_time_impl.get_local_time();
}

std::string ConsolePlatform::read_file(const std::string &path) {
	return fstream_file_impl.read_file(path);
}

void ConsolePlatform::write_file(const std::string &path, const std::string &contents) {
	return fstream_file_impl.write_file(path, contents);
}

void ConsolePlatform::debug_print(const std::string &msg) {
#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_DEBUG, "ConsolePlatform", (msg + "\n").c_str());
#else
	printf("%s\n", msg.c_str());
#endif
}

PlatformUtil::Promise<void> ConsolePlatform::delay(int ms) {
	auto pair = PlatformUtil::PromiseFactory::create<void>();
	timer_queue.insert(std::make_pair(get_timer() + ms, std::move(pair.second)));
	return std::move(pair.first);
}

PlatformUtil::Promise<void> ConsolePlatform::on_quit_request() {
	return on_quit_request_list.create_and_add();
}

PlatformUtil::Promise<void> ConsolePlatform::on_quit() {
	return on_quit_list.create_and_add();
}


std::unique_ptr<ConsolePlatform::BusSocket> ConsolePlatform::open_socket(const std::string &channel, uint32_t tid) {
	return bus_socket_impl.open_bus_socket(channel, tid);
}

PlatformUtil::Promise<short> ConsolePlatform::ConsoleFdPoller::on_fd_ready(int fd, short ev) {
	auto pair = PlatformUtil::PromiseFactory::create<short>();
	fds.push_back(std::make_pair(std::make_pair(fd, ev), std::move(pair.second)));
	return std::move(pair.first);
}

void ConsolePlatform::ConsoleFdPoller::poll(int timeout) {
	auto tmp = std::move(fds);
	fds.clear();
	tmp.erase(std::remove_if(tmp.begin(), tmp.end(), [](const auto &entry) { return !entry.second.is_pending(); }), tmp.end());

	std::vector<pollfd> pfd;
	pfd.reserve(tmp.size());
	for (const auto &entry : tmp)
		pfd.push_back({ entry.first.first, entry.first.second, 0 });

	::poll(pfd.data(), pfd.size(), timeout);

	for (int i = 0; i < pfd.size(); i++) {
		if (pfd[i].revents & POLLNVAL)
			platform->debug_print("nval!");
		else if ((pfd[i].events & pfd[i].revents) != 0 || (pfd[i].revents & (POLLERR | POLLHUP)) != 0)
			tmp[i].second.fulfill(pfd[i].revents);
		else
			fds.push_back(std::move(tmp[i]));
	}
}

bool ConsolePlatform::ConsoleFdPoller::is_empty() {
	return fds.empty();
}

void ConsolePlatform::event_loop() {
	while (running) {
		if (quit_request)
			on_quit_request_list.fulfill_all();

		{
			int64_t now = get_timer();
			std::vector<PlatformUtil::Fulfiller<void>> expired;
			while (!timer_queue.empty() && timer_queue.begin()->first <= now) {
				expired.push_back(std::move(timer_queue.begin()->second));
				timer_queue.erase(timer_queue.begin());
			}
			for (PlatformUtil::Fulfiller<void> &f : expired)
				f.fulfill();
		}

		int64_t diff = -1;
		if (!timer_queue.empty()) {
			diff = std::max((int64_t)0, timer_queue.begin()->first - get_timer());
		} else if (poller.is_empty()) {
			break;
		}

		poller.poll(diff);
	};

	on_quit_list.fulfill_all();
}

void ConsolePlatform::quit() {
	running = false;
}
