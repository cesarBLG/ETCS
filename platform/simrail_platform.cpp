 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "simrail_platform.h"
#include "platform_runtime.h"

using namespace PlatformUtil;

#define EXPORT_FUNC extern "C" __attribute__((visibility("default")))
#define IMPORT_FUNC(m, n) extern "C" __attribute__ ((import_module(m))) __attribute__ ((import_name(n)))

namespace api {
	IMPORT_FUNC("simrail_base_v1", "get_timer") int64_t get_timer();
	IMPORT_FUNC("simrail_base_v1", "get_timestamp") int64_t get_timestamp();
	IMPORT_FUNC("simrail_base_v1", "get_local_time") void get_local_time(int32_t* y, int32_t* m, int32_t* d, int32_t* hr, int32_t* min, int32_t* sec);
	IMPORT_FUNC("simrail_base_v1", "read_file") char* read_file(const char* t, size_t tlen, size_t* len);
	IMPORT_FUNC("simrail_base_v1", "write_file") uint32_t write_file(const char* t, size_t tlen, const char* c, size_t len);
	IMPORT_FUNC("simrail_base_v1", "debug_print") void debug_print(const char* t, size_t len);
	IMPORT_FUNC("simrail_base_v1", "delay") void delay(int32_t ms, void*, void*, void*);
	IMPORT_FUNC("simrail_base_v1", "on_quit_request") void on_quit_request(void*, void*, void*);
	IMPORT_FUNC("simrail_base_v1", "on_quit") void on_quit(void*, void*, void*);
	IMPORT_FUNC("simrail_base_v1", "quit") void quit();
	IMPORT_FUNC("simrail_base_v1", "open_socket") uint32_t open_socket(const char* name, size_t nlen, uint32_t tid);
}

extern "C" ssize_t writev(int fd, iovec *iov, int iovcnt) {
	ssize_t total = 0;
	for (int i = 0; i < iovcnt; i++) {
		api::debug_print((char*)iov[i].iov_base, iov[i].iov_len);
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
extern "C" int __isatty(int fd) {
    return 0;
}

EXPORT_FUNC void* alloc_mem(int size) {
    return malloc(size);
}

EXPORT_FUNC void free_mem(void *mem) {
    free(mem);
}

EXPORT_FUNC void init() {
#ifdef EVC
	platform = std::make_unique<SimrailBasePlatform>();
#else
	platform = std::make_unique<SimrailUiPlatform>(platform_size_w, platform_size_h);
#endif
	on_platform_ready();
}

static void callback_fulfill_void(Fulfiller<void>* fulfiller) {
	fulfiller->fulfill(false);
	while (DeferredFulfillment::execute());
	delete fulfiller;
}

static void callback_cancel_void(Fulfiller<void>* fulfiller) {
	delete fulfiller;
}

SimrailBasePlatform::SimrailBasePlatform() {
	PlatformUtil::DeferredFulfillment::list = &event_list;
}

int64_t SimrailBasePlatform::get_timer() {
	return api::get_timer();
}

int64_t SimrailBasePlatform::get_timestamp() {
	return api::get_timestamp();
}

BasePlatform::DateTime SimrailBasePlatform::get_local_time() {
	BasePlatform::DateTime datetime;
	api::get_local_time(&datetime.year, &datetime.month, &datetime.day, &datetime.hour, &datetime.minute, &datetime.second);
	return datetime;
}

std::optional<std::string> SimrailBasePlatform::read_file(const std::string_view path) {
	size_t len;
	char *ret = api::read_file(path.data(), path.size(), &len);
	if (!ret)
		return std::nullopt;
	std::string str(ret, len);
	free_mem(ret);
	return str;
}

bool SimrailBasePlatform::write_file(const std::string_view path, const std::string_view contents) {
	return api::write_file(path.data(), path.size(), contents.data(), contents.size()) == 1;
}

void SimrailBasePlatform::debug_print(const std::string_view msg) {
	api::debug_print(msg.data(), msg.size());
}

Promise<void> SimrailBasePlatform::delay(int ms) {
	auto pair = PromiseFactory::create<void>();
	api::delay(ms, (void*)&callback_fulfill_void, (void*)&callback_cancel_void, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}

Promise<void> SimrailBasePlatform::on_quit_request() {
	auto pair = PromiseFactory::create<void>();
	api::on_quit_request((void*)&callback_fulfill_void, (void*)&callback_cancel_void, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}

Promise<void> SimrailBasePlatform::on_quit() {
	auto pair = PromiseFactory::create<void>();
	api::on_quit((void*)&callback_fulfill_void, (void*)&callback_cancel_void, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}

std::unique_ptr<BasePlatform::BusSocket> SimrailBasePlatform::open_socket(const std::string_view channel, uint32_t tid) {
	uint32_t handle = api::open_socket(channel.data(), channel.size(), tid);
	if (!handle)
		return nullptr;
	return std::make_unique<SimrailBusSocket>(handle);
}

void SimrailBasePlatform::quit() {
	api::quit();
}

namespace api {
	IMPORT_FUNC("simrail_base_v1", "socket_close") void socket_close(uint32_t handle);
	IMPORT_FUNC("simrail_base_v1", "socket_broadcast") void socket_broadcast(uint32_t handle, const char* msg, size_t len);
	IMPORT_FUNC("simrail_base_v1", "socket_broadcast_tid") void socket_broadcast_tid(uint32_t handle, uint32_t tid, const char* msg, size_t len);
	IMPORT_FUNC("simrail_base_v1", "socket_send_to") void socket_send_to(uint32_t handle, uint32_t uid, const char* msg, size_t len);
	IMPORT_FUNC("simrail_base_v1", "socket_receive") void socket_receive(uint32_t handle, void*, void*, void*);
}

static void callback_fulfill_socket_receive(Fulfiller<BasePlatform::BusSocket::ReceiveResult>* fulfiller, uint32_t action, uint32_t tid, uint32_t uid, char* data, size_t len) {
	BasePlatform::BusSocket::PeerId peer { tid, uid };
	if (action == 1) {
		fulfiller->fulfill(BasePlatform::BusSocket::JoinNotification { peer }, false);
	} else if (action == 2) {
		fulfiller->fulfill(BasePlatform::BusSocket::LeaveNotification { peer }, false);
	} else if (action == 3) {
		fulfiller->fulfill(BasePlatform::BusSocket::Message { peer, std::string(data, len) }, false);
		free_mem(data);
	}
	while (DeferredFulfillment::execute());
	delete fulfiller;
}

static void callback_cancel_socket_receive(Fulfiller<BasePlatform::BusSocket::ReceiveResult>* fulfiller) {
	delete fulfiller;
}

SimrailBasePlatform::SimrailBusSocket::SimrailBusSocket(uint32_t handle) : handle(handle) {
}

SimrailBasePlatform::SimrailBusSocket::~SimrailBusSocket() {
	api::socket_close(handle);
}

void SimrailBasePlatform::SimrailBusSocket::broadcast(const std::string_view data) {
	api::socket_broadcast(handle, data.data(), data.size());
}

void SimrailBasePlatform::SimrailBusSocket::broadcast(uint32_t tid, const std::string_view data) {
	api::socket_broadcast_tid(handle, tid, data.data(), data.size());
}

void SimrailBasePlatform::SimrailBusSocket::send_to(uint32_t uid, const std::string_view data) {
	api::socket_send_to(handle, uid, data.data(), data.size());
}

PlatformUtil::Promise<BasePlatform::BusSocket::ReceiveResult> SimrailBasePlatform::SimrailBusSocket::receive() {
	auto pair = PromiseFactory::create<BasePlatform::BusSocket::ReceiveResult>();
	api::socket_receive(handle, (void*)&callback_fulfill_socket_receive, (void*)&callback_cancel_socket_receive, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}

namespace api {
	IMPORT_FUNC("simrail_ui_v1", "texture_create") uint32_t texture_create(uint32_t w, uint32_t h, void* data);
	IMPORT_FUNC("simrail_ui_v1", "texture_destroy") void texture_destroy(uint32_t handle);
	IMPORT_FUNC("simrail_ui_v1", "on_present_request") void on_present_request(void*, void*, void*);
	IMPORT_FUNC("simrail_ui_v1", "present") void present(ImDrawData* draw_data);
	IMPORT_FUNC("simrail_ui_v1", "on_input_event") void on_input_event(void*, void*, void*);
}

SimrailUiPlatform::SimrailUiPlatform(float virtual_w, float virtual_h) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(virtual_w, virtual_h);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.DeltaTime = 0.1f;
	io.BackendFlags = ImGuiBackendFlags_RendererHasVtxOffset;

	unsigned char *atlas_data;
	int atlas_x, atlas_y;
	io.Fonts->Flags = ImFontAtlasFlags_NoPowerOfTwoHeight;
	io.Fonts->GetTexDataAsRGBA32(&atlas_data, &atlas_x, &atlas_y);
	uint32_t tex_id = api::texture_create(atlas_x, atlas_y, atlas_data);
	io.Fonts->ClearTexData();
	io.Fonts->SetTexID((ImTextureID)tex_id);

	ImGui::NewFrame();
	drawlist = ImGui::GetBackgroundDrawList();

	input_promise = on_input_event().then(std::bind(&SimrailUiPlatform::handle_event, this, std::placeholders::_1));
}

SimrailUiPlatform::~SimrailUiPlatform() {
	ImGui::EndFrame();
	ImGui::DestroyContext();
}

void SimrailUiPlatform::handle_event(UiPlatform::InputEvent ev) {
	input_promise = on_input_event().then(std::bind(&SimrailUiPlatform::handle_event, this, std::placeholders::_1));
	pending_events.push_back(ev);
}

static void callback_fulfill_input_event(Fulfiller<UiPlatform::InputEvent>* fulfiller, uint32_t action, float x, float y) {
	fulfiller->fulfill(UiPlatform::InputEvent { (UiPlatform::InputEvent::Action)action, x * platform_size_w, y * platform_size_h }, false);
	while (DeferredFulfillment::execute());
	delete fulfiller;
}

static void callback_cancel_input_event(Fulfiller<UiPlatform::InputEvent>* fulfiller) {
	delete fulfiller;
}

void SimrailUiPlatform::set_color(Color c) {
	current_color = IM_COL32(c.R, c.G, c.B, 255);
}

void SimrailUiPlatform::draw_line(float x1, float y1, float x2, float y2) {
	drawlist->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), current_color);
}

void SimrailUiPlatform::draw_rect(float x, float y, float w, float h) {
	drawlist->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), current_color);
}

void SimrailUiPlatform::draw_rect_filled(float x, float y, float w, float h) {
	drawlist->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), current_color);
}

void SimrailUiPlatform::draw_image(const Image &base, float x, float y, float w, float h) {
}

void SimrailUiPlatform::draw_circle_filled(float x, float y, float rad) {
	drawlist->AddCircleFilled(ImVec2(x, y), rad, current_color);
}

void SimrailUiPlatform::draw_polygon_filled(const std::vector<std::pair<float, float>> &poly) {
	for (const auto &p : poly)
		drawlist->PathLineTo(ImVec2(p.first, p.second));
	drawlist->PathFillConvex(current_color);
}

Promise<void> SimrailUiPlatform::on_present_request() {
	auto pair = PromiseFactory::create<void>();
	api::on_present_request((void*)&callback_fulfill_void, (void*)&callback_cancel_void, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}

void SimrailUiPlatform::present() {
	ImGui::Render();
	api::present(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	for (InputEvent &ev : pending_events) {
		io.AddMousePosEvent(ev.x, ev.y);
		io.AddMouseButtonEvent(0, ev.action != InputEvent::Action::Release);
	}
	pending_events.clear();

	ImGui::NewFrame();
	drawlist = ImGui::GetBackgroundDrawList();
	ImGui::ShowDemoWindow();
}

std::unique_ptr<SimrailUiPlatform::Image> SimrailUiPlatform::load_image(const std::string_view p) {
	return std::make_unique<SimrailImage>();
}

std::unique_ptr<SimrailUiPlatform::Font> SimrailUiPlatform::load_font(float size, bool bold, const std::string_view lang) {
	return std::make_unique<SimrailFont>();
}

std::unique_ptr<SimrailUiPlatform::Image> SimrailUiPlatform::make_text_image(const std::string_view text, const Font &base, Color c) {
	return std::make_unique<SimrailImage>();
}

std::unique_ptr<SimrailUiPlatform::Image> SimrailUiPlatform::make_wrapped_text_image(const std::string_view text, const Font &base, int align, Color c) {
	return std::make_unique<SimrailImage>();
}

void SimrailUiPlatform::set_volume(int vol) {
}

int SimrailUiPlatform::get_volume() {
	return 50;
}

std::unique_ptr<SimrailUiPlatform::SoundData> SimrailUiPlatform::load_sound(const std::string_view path) {
	return std::make_unique<SimrailSoundData>();
}

std::unique_ptr<SimrailUiPlatform::SoundData> SimrailUiPlatform::load_sound(const std::vector<std::pair<int, int>> &melody) {
	return std::make_unique<SimrailSoundData>();
}

std::unique_ptr<SimrailUiPlatform::SoundSource> SimrailUiPlatform::play_sound(const SoundData &base, bool looping) {
	return std::make_unique<SimrailSoundSource>();
}

void SimrailUiPlatform::set_brightness(int vol) {

}

int SimrailUiPlatform::get_brightness() {
	return 0;
}

Promise<UiPlatform::InputEvent> SimrailUiPlatform::on_input_event() {
	auto pair = PromiseFactory::create<UiPlatform::InputEvent>();
	api::on_input_event((void*)&callback_fulfill_input_event, (void*)&callback_cancel_void, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}
