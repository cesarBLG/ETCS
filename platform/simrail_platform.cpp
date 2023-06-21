 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "simrail_platform.h"
#include <algorithm>
#include <chrono>
#include <fstream>

#define EXPORT_FUNC extern "C" __attribute__((visibility("default")))
#define IMPORT_FUNC(n) extern "C" __attribute__ ((import_name(n)))

EXPORT_FUNC void* alloc_mem(int size)
{
    return malloc(size);
}

EXPORT_FUNC void free_mem(void *mem)
{
    free(mem);
}

EXPORT_FUNC void callback_void(PlatformUtil::Fulfiller<void>* fulfiller)
{
	fulfiller->fulfill();
	delete fulfiller;
}

EXPORT_FUNC void callback_inputevent(PlatformUtil::Fulfiller<SimrailPlatform::InputEvent>* fulfiller, SimrailPlatform::InputEvent::Action action, float x, float y)
{
	fulfiller->fulfill(SimrailPlatform::InputEvent{action, x, y});
	delete fulfiller;
}

IMPORT_FUNC("get_timer") int64_t get_timer();
IMPORT_FUNC("get_timestamp") int64_t get_timestamp();
IMPORT_FUNC("read_file") char* read_file(const char* t, size_t* len);
IMPORT_FUNC("plugin_log") void plugin_log(const char* t);
IMPORT_FUNC("plugin_log_len") void plugin_log_len(const char* t, size_t len);
IMPORT_FUNC("delay") void delay(int32_t ms, void *ticket);
IMPORT_FUNC("on_input_event") void on_input_event(void *ticket);

extern "C" ssize_t writev(int fd, struct iovec *iov, int iovcnt) {
	ssize_t total = 0;
	for (int i = 0; i < iovcnt; i++) {
		::plugin_log_len((char*)iov[i].iov_base, iov[i].iov_len);
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

SimrailPlatform::SimrailPlatform(float virtual_w, float virtual_h) {
}

SimrailPlatform::~SimrailPlatform() {
}

int64_t SimrailPlatform::get_timer() {
	return ::get_timer();
}

int64_t SimrailPlatform::get_timestamp() {
	return ::get_timestamp();
}

SimrailPlatform::DateTime SimrailPlatform::get_local_time() {
	return {};
}

std::string SimrailPlatform::read_file(const std::string &path) {
	size_t len;
	char *ret = ::read_file(path.c_str(), &len);
	if (!ret)
		return "";
	std::string str(ret, len);
	::free_mem(ret);
	return str;
}

void SimrailPlatform::debug_print(const std::string &msg) {
	::plugin_log_len(msg.data(), msg.size());
}

PlatformUtil::Promise<void> SimrailPlatform::delay(int ms) {
	auto pair = PlatformUtil::PromiseFactory::create<void>();
	::delay(ms, new PlatformUtil::Fulfiller<void>(std::move(pair.second)));
	return std::move(pair.first);
}

PlatformUtil::Promise<void> SimrailPlatform::on_quit_request() {
	return {};
}

PlatformUtil::Promise<void> SimrailPlatform::on_quit() {
	return {};
}

std::unique_ptr<SimrailPlatform::BusSocket> SimrailPlatform::open_socket(const std::string &channel, uint32_t) {
	return nullptr;
}

PlatformUtil::Promise<SimrailPlatform::InputEvent> SimrailPlatform::on_input_event() {
	auto pair = PlatformUtil::PromiseFactory::create<InputEvent>();
	::on_input_event(new PlatformUtil::Fulfiller<InputEvent>(std::move(pair.second)));
	return std::move(pair.first);
}

void SimrailPlatform::event_loop() {
}

void SimrailPlatform::quit() {
}

void SimrailPlatform::set_color(Color c) {
}

void SimrailPlatform::draw_line(float x1, float y1, float x2, float y2) {
}

void SimrailPlatform::draw_rect(float x, float y, float w, float h) {
}

void SimrailPlatform::draw_rect_filled(float x, float y, float w, float h) {
}

void SimrailPlatform::draw_image(const Image &base, float x, float y, float w, float h) {
}

void SimrailPlatform::draw_circle_filled(float x, float y, float rad) {
}

void SimrailPlatform::draw_polygon_filled(const std::vector<std::pair<float, float>> &poly) {
}

void SimrailPlatform::clear() {
}

PlatformUtil::Promise<void> SimrailPlatform::present() {
	return {};
}

std::unique_ptr<SimrailPlatform::Image> SimrailPlatform::load_image(const std::string &p) {
	return nullptr;
}

std::unique_ptr<SimrailPlatform::Font> SimrailPlatform::load_font(float size, bool bold) {
	return nullptr;
}

std::unique_ptr<SimrailPlatform::Image> SimrailPlatform::make_text_image(const std::string &text, const Font &base, Color c) {
	return nullptr;
}

std::unique_ptr<SimrailPlatform::Image> SimrailPlatform::make_wrapped_text_image(const std::string &text, const Font &base, int align, Color c) {
	return nullptr;
}

void SimrailPlatform::set_volume(int vol) {
}

std::unique_ptr<SimrailPlatform::SoundData> SimrailPlatform::load_sound(const std::string &path) {
	return nullptr;
}

std::unique_ptr<SimrailPlatform::SoundData> SimrailPlatform::load_sound(const std::vector<std::pair<int, int>> &melody) {
	return nullptr;
}

std::unique_ptr<SimrailPlatform::SoundSource> SimrailPlatform::play_sound(const SoundData &base, bool looping) {
	return nullptr;
}

void SimrailPlatform::set_brightness(int vol) {

}
