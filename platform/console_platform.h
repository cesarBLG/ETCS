 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include <atomic>
#include <optional>
#include <functional>
#include "platform.h"
#include "bus_socket_impl.h"
#include "libc_time_impl.h"
#include "fstream_file_impl.h"
#include "bus_socket_impl.h"
#include "bus_socket_server.h"
#include "bus_tcp_bridge.h"
#include "console_fd_poller.h"

class ConsolePlatform final : public BasePlatform {
private:
	std::string load_path;
	bool running;
	std::atomic<bool> quit_request;

	PlatformUtil::FulfillerList<void> on_quit_request_list;
	PlatformUtil::FulfillerList<void> on_quit_list;
	std::multimap<int, PlatformUtil::Fulfiller<void>> timer_queue;

	ConsoleFdPoller poller;

	BusSocketImpl bus_socket_impl;
	LibcTimeImpl libc_time_impl;
	FstreamFileImpl fstream_file_impl;
#ifdef EVC
	BusSocketServerManager bus_server_manager;
	BusTcpBridgeManager bus_bridge_manager;
#endif
	std::vector<std::unique_ptr<PlatformUtil::TypeErasedFulfiller>> event_list;

public:
	ConsolePlatform(const std::string_view load_path, const std::vector<std::string> &args);
	void event_loop();
	ConsoleFdPoller& get_poller();

	~ConsolePlatform() override;

	int64_t get_timer() override;
	int64_t get_timestamp() override;
	DateTime get_local_time() override;

	std::unique_ptr<BusSocket> open_socket(const std::string_view channel, uint32_t tid) override;
	std::optional<std::string> read_file(const std::string_view path) override;
	bool write_file(const std::string_view path, const std::string_view contents) override;
	void debug_print(const std::string_view msg) override;

	PlatformUtil::Promise<void> delay(int ms) override;
	PlatformUtil::Promise<void> on_quit_request() override;
	PlatformUtil::Promise<void> on_quit() override;

	void quit() override;
};
