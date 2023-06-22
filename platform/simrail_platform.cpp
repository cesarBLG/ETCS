 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "simrail_platform.h"

#define EXPORT_FUNC extern "C" __attribute__((visibility("default")))
#define IMPORT_FUNC(m, n) extern "C" __attribute__ ((import_module(m))) __attribute__ ((import_name(n)))

EXPORT_FUNC void* alloc_mem(int size) {
    return malloc(size);
}

EXPORT_FUNC void free_mem(void *mem) {
    free(mem);
}

using namespace PlatformUtil;

void callback_fulfill_void(Fulfiller<void>* fulfiller) {
	fulfiller->fulfill(false);
	while (DeferredFulfillment::execute());
	delete fulfiller;
}

void callback_cancel_void(Fulfiller<void>* fulfiller) {
	delete fulfiller;
}

IMPORT_FUNC("simrail", "get_timer") int64_t get_timer();
IMPORT_FUNC("simrail", "get_timestamp") int64_t get_timestamp();
IMPORT_FUNC("simrail", "read_file") char* read_file(const char* t, size_t* len);
IMPORT_FUNC("simrail", "write_file") void write_file(const char* t, const char* c, size_t len);
IMPORT_FUNC("simrail", "debug_print") void debug_print(const char* t, size_t len);
IMPORT_FUNC("simrail", "delay") void delay(int32_t ms, void*, void*, void*);
IMPORT_FUNC("simrail", "on_quit_request") void on_quit_request(void*, void*, void*);
IMPORT_FUNC("simrail", "on_quit") void on_quit(void*, void*, void*);
IMPORT_FUNC("simrail", "quit") void quit();
IMPORT_FUNC("simrail", "open_socket") uint32_t open_socket(const char* name, uint32_t tid);

extern "C" ssize_t writev(int fd, iovec *iov, int iovcnt) {
	ssize_t total = 0;
	for (int i = 0; i < iovcnt; i++) {
		::debug_print((char*)iov[i].iov_base, iov[i].iov_len);
		total += iov[i].iov_len;
	}
    return total;
}
extern "C" int64_t __lseek(int fd, int64_t off, int whence) {
    return 0;
}
extern "C" int close(int fd) {
    return 0;
}

EXPORT_FUNC void init() {
#ifdef EVC
	platform = std::make_unique<SimrailBasePlatform>();
#else
	platform = std::make_unique<SimrailUiPlatform>();
#endif
	on_platform_ready();
}

SimrailBasePlatform::SimrailBasePlatform() {
}

int64_t SimrailBasePlatform::get_timer() {
	return ::get_timer();
}

int64_t SimrailBasePlatform::get_timestamp() {
	return ::get_timestamp();
}

BasePlatform::DateTime SimrailBasePlatform::get_local_time() {
	return {};
}

std::string SimrailBasePlatform::read_file(const std::string &path) {
	size_t len;
	char *ret = ::read_file(path.c_str(), &len);
	if (!ret)
		return "";
	std::string str(ret, len);
	::free_mem(ret);
	return str;
}

void SimrailBasePlatform::write_file(const std::string &path, const std::string &contents) {
	::write_file(path.c_str(), contents.data(), contents.size());
}

void SimrailBasePlatform::debug_print(const std::string &msg) {
	::debug_print(msg.data(), msg.size());
}

Promise<void> SimrailBasePlatform::delay(int ms) {
	auto pair = PromiseFactory::create<void>();
	::delay(ms, (void*)&callback_fulfill_void, (void*)&callback_cancel_void, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}

Promise<void> SimrailBasePlatform::on_quit_request() {
	auto pair = PromiseFactory::create<void>();
	::on_quit_request((void*)&callback_fulfill_void, (void*)&callback_cancel_void, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}

Promise<void> SimrailBasePlatform::on_quit() {
	auto pair = PromiseFactory::create<void>();
	::on_quit((void*)&callback_fulfill_void, (void*)&callback_cancel_void, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}

std::unique_ptr<BasePlatform::BusSocket> SimrailBasePlatform::open_socket(const std::string &channel, uint32_t tid) {
	uint32_t handle = ::open_socket(channel.c_str(), tid);
	if (!handle)
		return nullptr;
	return std::make_unique<SimrailBusSocket>(handle);
}

void SimrailBasePlatform::quit() {
	::quit();
}

IMPORT_FUNC("simrail", "socket_close") void socket_close(uint32_t handle);
IMPORT_FUNC("simrail", "socket_broadcast") void socket_broadcast(uint32_t handle, const char* msg, size_t len);
IMPORT_FUNC("simrail", "socket_broadcast_tid") void socket_broadcast_tid(uint32_t handle, uint32_t tid, const char* msg, size_t len);
IMPORT_FUNC("simrail", "socket_send_to") void socket_send_to(uint32_t handle, uint32_t uid, const char* msg, size_t len);
IMPORT_FUNC("simrail", "socket_receive") void socket_receive(uint32_t handle, void*, void*, void*);
IMPORT_FUNC("simrail", "socket_on_peer_join") void socket_on_peer_join(uint32_t handle, void*, void*, void*);
IMPORT_FUNC("simrail", "socket_on_peer_leave") void socket_on_peer_leave(uint32_t handle, void*, void*, void*);

void callback_fulfill_socket_receive(Fulfiller<std::pair<BasePlatform::BusSocket::PeerId, std::string>>* fulfiller, uint32_t tid, uint32_t uid, char* data, size_t len) {
	fulfiller->fulfill(std::make_pair(BasePlatform::BusSocket::PeerId{ tid, uid }, std::string(data, len)), false);
	::free_mem(data);
	while (DeferredFulfillment::execute());
	delete fulfiller;
}

void callback_cancel_socket_receive(Fulfiller<std::pair<BasePlatform::BusSocket::PeerId, std::string>>* fulfiller) {
	delete fulfiller;
}

void callback_fulfill_socket_peer(Fulfiller<BasePlatform::BusSocket::PeerId>* fulfiller, uint32_t tid, uint32_t uid) {
	fulfiller->fulfill(BasePlatform::BusSocket::PeerId{ tid, uid }, false);
	delete fulfiller;
	while (DeferredFulfillment::execute());
}

void callback_cancel_socket_peer(Fulfiller<BasePlatform::BusSocket::PeerId>* fulfiller) {
	delete fulfiller;
}

SimrailBasePlatform::SimrailBusSocket::SimrailBusSocket(uint32_t handle) : handle(handle) {
}

SimrailBasePlatform::SimrailBusSocket::~SimrailBusSocket() {
	::socket_close(handle);
}

void SimrailBasePlatform::SimrailBusSocket::broadcast(const std::string &data) {
	::socket_broadcast(handle, data.data(), data.size());
}

void SimrailBasePlatform::SimrailBusSocket::broadcast(uint32_t tid, const std::string &data) {
	::socket_broadcast_tid(handle, tid, data.data(), data.size());
}

void SimrailBasePlatform::SimrailBusSocket::send_to(uint32_t uid, const std::string &data) {
	::socket_send_to(handle, uid, data.data(), data.size());
}

Promise<std::pair<BasePlatform::BusSocket::PeerId, std::string>> SimrailBasePlatform::SimrailBusSocket::receive() {
	auto pair = PromiseFactory::create<std::pair<BusSocket::PeerId, std::string>>();
	::socket_receive(handle, (void*)&callback_fulfill_socket_receive, (void*)&callback_cancel_socket_receive, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}

Promise<BasePlatform::BusSocket::PeerId> SimrailBasePlatform::SimrailBusSocket::on_peer_join() {
	auto pair = PromiseFactory::create<BasePlatform::BusSocket::PeerId>();
	::socket_on_peer_join(handle, (void*)&callback_fulfill_socket_peer, (void*)&callback_cancel_socket_peer, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}

Promise<BasePlatform::BusSocket::PeerId> SimrailBasePlatform::SimrailBusSocket::on_peer_leave() {
	auto pair = PromiseFactory::create<BasePlatform::BusSocket::PeerId>();
	::socket_on_peer_leave(handle, (void*)&callback_fulfill_socket_peer, (void*)&callback_cancel_socket_peer, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}

//IMPORT_FUNC("simrail", "on_input_event") void on_input_event(void *ticket);

/*
EXPORT_FUNC void callback_inputevent(Fulfiller<UiPlatform::InputEvent>* fulfiller, UiPlatform::InputEvent::Action action, float x, float y)
{
	fulfiller->fulfill(UiPlatform::InputEvent{action, x, y}, false);
	delete fulfiller;
	while (DeferredFulfillment::execute());
}
*/

void SimrailUiPlatform::set_color(Color c) {
}

void SimrailUiPlatform::draw_line(float x1, float y1, float x2, float y2) {
}

void SimrailUiPlatform::draw_rect(float x, float y, float w, float h) {
}

void SimrailUiPlatform::draw_rect_filled(float x, float y, float w, float h) {
}

void SimrailUiPlatform::draw_image(const Image &base, float x, float y, float w, float h) {
}

void SimrailUiPlatform::draw_circle_filled(float x, float y, float rad) {
}

void SimrailUiPlatform::draw_polygon_filled(const std::vector<std::pair<float, float>> &poly) {
}

void SimrailUiPlatform::clear() {
}

Promise<void> SimrailUiPlatform::present() {
	return {};
}

std::unique_ptr<SimrailUiPlatform::Image> SimrailUiPlatform::load_image(const std::string &p) {
	return nullptr;
}

std::unique_ptr<SimrailUiPlatform::Font> SimrailUiPlatform::load_font(float size, bool bold) {
	return nullptr;
}

std::unique_ptr<SimrailUiPlatform::Image> SimrailUiPlatform::make_text_image(const std::string &text, const Font &base, Color c) {
	return nullptr;
}

std::unique_ptr<SimrailUiPlatform::Image> SimrailUiPlatform::make_wrapped_text_image(const std::string &text, const Font &base, int align, Color c) {
	return nullptr;
}

void SimrailUiPlatform::set_volume(int vol) {
}

std::unique_ptr<SimrailUiPlatform::SoundData> SimrailUiPlatform::load_sound(const std::string &path) {
	return nullptr;
}

std::unique_ptr<SimrailUiPlatform::SoundData> SimrailUiPlatform::load_sound(const std::vector<std::pair<int, int>> &melody) {
	return nullptr;
}

std::unique_ptr<SimrailUiPlatform::SoundSource> SimrailUiPlatform::play_sound(const SoundData &base, bool looping) {
	return nullptr;
}

void SimrailUiPlatform::set_brightness(int vol) {

}

Promise<UiPlatform::InputEvent> SimrailUiPlatform::on_input_event() {
	return {};
	/*
	auto pair = PromiseFactory::create<InputEvent>();
	::on_input_event(new Fulfiller<InputEvent>(std::move(pair.second)));
	return std::move(pair.first);
	*/
}
