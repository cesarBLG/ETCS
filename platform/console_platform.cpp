 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "console_platform.h"
#include "platform_runtime.h"
#include <iostream>

#ifdef __unix__
#include <signal.h>
#endif
#ifdef __ANDROID__
#include <android/log.h>
#endif

static std::atomic<bool>* quit_request_ptr;

static void sigterm_handler(int sig) {
	*quit_request_ptr = true;
}

#ifdef __ANDROID__
#include <jni.h>
extern "C" void Java_com_etcs_dmi_EVC_evcMain(JNIEnv *env, jobject thiz, jstring stringObject)
{
    jboolean b;
	std::vector<std::string> args;
	platform = std::make_unique<ConsolePlatform>(std::string(env->GetStringUTFChars(stringObject, &b)) + "/", args);
	on_platform_ready();
	static_cast<ConsolePlatform*>(platform.get())->event_loop();
}
extern "C" void Java_com_etcs_dmi_EVC_evcStop(JNIEnv *env, jobject thiz)
{
    sigterm_handler(SIGTERM);
}
#else
int main(int argc, char *argv[])
{
	std::vector<std::string> args;
	for (int i = 1; i < argc; i++)
		args.push_back(std::string(argv[i]));
	platform = std::make_unique<ConsolePlatform>("", args);
	on_platform_ready();
	static_cast<ConsolePlatform*>(platform.get())->event_loop();
	return 0;
}
#endif
ConsolePlatform::ConsolePlatform(const std::string_view path, const std::vector<std::string> &args) :
	load_path(path),
	bus_socket_impl(load_path, poller, args),
	fstream_file_impl(load_path)
#ifdef EVC
	,
	bus_server_manager(load_path, poller),
	bus_bridge_manager(load_path, poller, bus_socket_impl),
	orts_bridge(load_path, poller, bus_socket_impl)
#endif
	{
	running = true;
	quit_request = false;
#ifdef __unix__
	quit_request_ptr = &quit_request;
	signal(SIGTERM, &sigterm_handler);
	signal(SIGINT, &sigterm_handler);
#endif
	PlatformUtil::DeferredFulfillment::list = &event_list;
}

ConsolePlatform::~ConsolePlatform() {
	on_quit_request_list.clear();
	on_quit_list.clear();
	timer_queue.clear();
	while (PlatformUtil::DeferredFulfillment::execute());
	PlatformUtil::DeferredFulfillment::list = nullptr;

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

std::optional<std::string> ConsolePlatform::read_file(const std::string_view path) {
	return fstream_file_impl.read_file(path);
}

bool ConsolePlatform::write_file(const std::string_view path, const std::string_view contents) {
	return fstream_file_impl.write_file(path, contents);
}

void ConsolePlatform::debug_print(const std::string_view msg) {
#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_DEBUG, "ConsolePlatform" ,"%s\n", std::string(msg).c_str());
#else
	std::cout << msg << std::endl;
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


std::unique_ptr<ConsolePlatform::BusSocket> ConsolePlatform::open_socket(const std::string_view channel, uint32_t tid) {
	return bus_socket_impl.open_bus_socket(channel, tid);
}

ConsoleFdPoller& ConsolePlatform::get_poller() {
	return poller;
}

void ConsolePlatform::event_loop() {
	while (running) {
		bool idle = true;

		if (quit_request) {
			idle = false;
			on_quit_request_list.fulfill_all(false);
		}

		int64_t now = get_timer();
		if (!timer_queue.empty() && timer_queue.begin()->first <= now) {
			idle = false;
			timer_queue.begin()->second.fulfill(false);
			timer_queue.erase(timer_queue.begin());
		}

		for (int i = 0; i < 10; i++)
			if (PlatformUtil::DeferredFulfillment::execute())
				idle = false;
			else
				break;

		int64_t diff = -1;
		if (!timer_queue.empty())
			diff = std::max((int64_t)0, timer_queue.begin()->first - get_timer());

		poller.poll(idle ? diff : 0);
	};

	on_quit_list.fulfill_all(false);
}

void ConsolePlatform::quit() {
	running = false;
}
