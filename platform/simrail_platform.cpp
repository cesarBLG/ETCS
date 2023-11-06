 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "simrail_platform.h"
#include "platform_runtime.h"
#include "stb/stb_image.h"

using namespace PlatformUtil;

#define EXPORT_FUNC extern "C" __attribute__((visibility("default")))
#define IMPORT_FUNC(m, n) extern "C" __attribute__ ((import_module(m))) __attribute__ ((import_name(n)))

namespace api {
	IMPORT_FUNC("simrail_base_v1", "get_timer") int64_t get_timer();
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
	IMPORT_FUNC("simrail_ui_v1", "set_brightness") void set_brightness(float br);
}

SimrailUiPlatform::SimrailUiPlatform(float virtual_w, float virtual_h) {
	IMGUI_CHECKVERSION();
	build_atlas();
	current_atlas = std::move(pending_atlas);
	ImGui::CreateContext(current_atlas.get());

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(virtual_w, virtual_h);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.DeltaTime = 0.1f;
	io.BackendFlags = ImGuiBackendFlags_RendererHasVtxOffset;

	atlas_id = 0;
	current_color = 0;
	upload_atlas();

	ImGui::NewFrame();
	drawlist = ImGui::GetBackgroundDrawList();

	input_promise = on_input_event().then(std::bind(&SimrailUiPlatform::handle_event, this, std::placeholders::_1));

	set_volume(50);
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
	fulfiller->fulfill(UiPlatform::InputEvent { (UiPlatform::InputEvent::Action)action, x * platform_size_w, (1.0f - y) * platform_size_h }, false);
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

void SimrailUiPlatform::draw_image(const Image &base, float x, float y) {
	const SimrailImage &img = dynamic_cast<const SimrailImage&>(base);
	if (img.image) {
		if (!img.image->pending)
			build_atlas();
		if (!img.image->current)
			return;
		drawlist->AddImage((ImTextureID)atlas_id,
			ImVec2(x, y),
			ImVec2(x + img.image->width, y + img.image->height),
			ImVec2(-img.image->current->uv0x, -img.image->current->uv0y),
			ImVec2(-img.image->current->uv1x, -img.image->current->uv1y));
	} else if (img.font) {
		if (!img.font->font->current)
			return;
		float adjusted_size = img.font->font->size * (img.font->ascent / img.font->font->current->Ascent);
		drawlist->AddText(img.font->font->current, adjusted_size, ImVec2(x, y), img.color, img.text.data(), img.text.data() + img.text.size());
	}
}

void SimrailUiPlatform::draw_circle_filled(float x, float y, float r) {
	drawlist->AddCircleFilled(ImVec2(x, y), r, current_color);
}

void SimrailUiPlatform::draw_arc_filled(float x, float y, float r_min, float r_max, float a_min, float a_max) {
	float arc_angle = a_max - a_min;

	int circle_segments = drawlist->_CalcCircleAutoSegmentCount(r_max);
	int arc_segments = std::max((int)std::ceil(circle_segments * arc_angle / (M_PI * 2.0f)), (int)(2.0f * M_PI / arc_angle));

	for (int i = 0; i < arc_segments; i++) {
		float a1 = a_min + ((float)i / arc_segments) * arc_angle;
		float a2 = a_min + ((float)(i + 1) / arc_segments) * arc_angle;

		drawlist->AddQuadFilled(
			ImVec2(x + r_max * std::cos(a1), y + r_max * std::sin(a1)),
			ImVec2(x + r_min * std::cos(a1), y + r_min * std::sin(a1)),
			ImVec2(x + r_min * std::cos(a2), y + r_min * std::sin(a2)),
			ImVec2(x + r_max * std::cos(a2), y + r_max * std::sin(a2)),
			current_color);
	}
}

void SimrailUiPlatform::draw_convex_polygon_filled(const std::vector<std::pair<float, float>> &poly) {
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
	ImGuiIO& io = ImGui::GetIO();

	ImGui::EndFrame();

	if (!pending_atlas) {
		ImGui::Render();
		api::present(ImGui::GetDrawData());
	}

	if (pending_atlas) {
		current_atlas = std::move(pending_atlas);
		io.Fonts = current_atlas.get();
		upload_atlas();
	}

	for (InputEvent &ev : pending_events) {
		io.AddMousePosEvent(ev.x, ev.y);
		io.AddMouseButtonEvent(0, ev.action != InputEvent::Action::Release);
	}
	pending_events.clear();

	ImGui::NewFrame();
	drawlist = ImGui::GetBackgroundDrawList();
}

void SimrailUiPlatform::build_atlas() {
	pending_atlas = std::make_unique<ImFontAtlas>();

	int max_width = 0;
	int rect_it = pending_atlas->CustomRects.size();
	for (auto const &entry : loaded_images) {
		max_width = std::max(max_width, entry.second->width);
		pending_atlas->AddCustomRectRegular(entry.second->width, entry.second->height);
	}

	ImFontConfig config;
	config.FontDataOwnedByAtlas = false;

	ImVector<ImWchar> ranges;
	std::vector<size_t> ranges_off;
	ranges_off.reserve(loaded_fonts.size());

	for (auto const &entry : loaded_fonts) {
		ranges_off.push_back(ranges.size());
		entry.second->ranges_builder.BuildRanges(&ranges);
	}

	auto ranges_it = ranges_off.begin();
	for (auto const &entry : loaded_fonts) {
		config.MergeMode = false;
		for (auto const &data : entry.second->font_data) {
			entry.second->pending = pending_atlas->AddFontFromMemoryTTF(data->data(), data->size(), entry.second->size, &config, &ranges[*ranges_it]);
			config.MergeMode = true;
		}
		++ranges_it;
	}

	pending_atlas->Flags = ImFontAtlasFlags_NoPowerOfTwoHeight | ImFontAtlasFlags_NoMouseCursors;
	pending_atlas->TexDesiredWidth = std::max(512, max_width);
	pending_atlas->Build();

	for (auto const &entry : loaded_images) {
		ImFontAtlasCustomRect &rect = pending_atlas->CustomRects[rect_it++];
		ImVec2 uv0, uv1;
		pending_atlas->CalcCustomRectUV(&rect, &uv0, &uv1);
		entry.second->pending.emplace(AtlasRect { rect.X, rect.Y, uv0.x, uv0.y, uv1.x, uv1.y });
	}
}

void SimrailUiPlatform::upload_atlas() {
	if (atlas_id != 0)
		api::texture_destroy(atlas_id);

	unsigned char *atlas_data;
	int atlas_x, atlas_y;
	current_atlas->GetTexDataAsRGBA32(&atlas_data, &atlas_x, &atlas_y);

	for (auto const &entry : loaded_images) {
		entry.second->current = entry.second->pending;

		for (int y = 0; y < entry.second->height; y++)
			memcpy(&atlas_data[((entry.second->current->y + y) * atlas_x + entry.second->current->x) * 4], entry.second->data.get() + y * entry.second->width * 4, entry.second->width * 4);
	}

	for (auto const &entry : loaded_fonts)
		entry.second->current = entry.second->pending;

	atlas_id = api::texture_create(atlas_x, atlas_y, atlas_data);

	current_atlas->ClearTexData();
	current_atlas->SetTexID((ImTextureID)atlas_id);
}

std::unique_ptr<SimrailUiPlatform::Image> SimrailUiPlatform::load_image(const std::string_view path) {
	std::shared_ptr<SimrailImageWrapper> wrapper;
	{
		auto it = loaded_images.find(path);
		if (it != loaded_images.end())
			wrapper = it->second;
	}

	if (!wrapper) {
		std::optional<std::string> data = read_file(path);
		if (!data)
			return nullptr;

		int x, y, channels;
		uint8_t *ret = stbi_load_from_memory((uint8_t*)data->data(), data->size(), &x, &y, &channels, 4);
		if (!ret)
			return nullptr;

		wrapper = std::make_unique<SimrailImageWrapper>();
		wrapper->data = std::unique_ptr<uint8_t[], stbi_deleter>(ret);
		wrapper->width = x;
		wrapper->height = y;

		loaded_images.insert(std::make_pair(path, wrapper));
	}

	auto image = std::make_unique<SimrailImage>();
	image->image = std::move(wrapper);

	return image;
}

IMGUI_API int ImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end);

static bool glyph_ranges_set_bit(ImFontGlyphRangesBuilder &builder, size_t n)
{
	int off = (int)(n >> 5);
	ImU32 mask = 1u << (n & 31);
	if ((builder.UsedChars[off] & mask) == 0) {
		builder.UsedChars[off] |= mask;
		return true;
	}
	return false;
}

static bool glyph_ranges_add(ImFontGlyphRangesBuilder &builder, const char *text, const char* text_end)
{
	bool ret = false;
	while (text < text_end) {
		unsigned int c = 0;
		int c_len = ImTextCharFromUtf8(&c, text, text_end);
		text += c_len;
		if (c_len == 0)
			break;
		ret |= glyph_ranges_set_bit(builder, c);
	}
	return ret;
}

SimrailUiPlatform::SimrailFont::SimrailFont(std::shared_ptr<SimrailFontWrapper> wrapper, float s, SimrailUiPlatform &p) : font(std::move(wrapper)), ascent(s), platform(p) {

}

std::pair<float, float> SimrailUiPlatform::SimrailFont::calc_size(const std::string_view str) const {
	bool dirty = glyph_ranges_add(font->ranges_builder, str.begin(), str.end());
	if (dirty || !font->pending)
		platform.build_atlas();
	float adjusted_size = font->size * (ascent / font->pending->Ascent);
	ImVec2 ret = font->pending->CalcTextSizeA(adjusted_size, FLT_MAX, 0.0f, str.begin(), str.end());
	return std::make_pair(ret.x, ret.y);
}

void SimrailUiPlatform::stbi_deleter::operator()(uint8_t *ptr) {
	stbi_image_free(ptr);
}

std::pair<float, float> SimrailUiPlatform::SimrailImage::size() const {
	if (image)
		return std::make_pair(image->width, image->height);
	else if (font)
		return text_size;
	return {};
}

std::unique_ptr<SimrailUiPlatform::Font> SimrailUiPlatform::load_font(float ascent, bool bold, const std::string_view lang) {
#ifdef SIMRAIL
	ascent *= 1.1f;
#endif

	std::string lang_str(lang);
	std::shared_ptr<SimrailFontWrapper> wrapper;

	if (lang_str != "zh_Hans" && lang_str != "zh_Hant")
		lang_str = "default";

	float coarse_size = std::ceil(ascent / 5.0f) * 10.0f;

	{
		auto it = loaded_fonts.find({ coarse_size, bold, lang_str });
		if (it != loaded_fonts.end())
			wrapper = it->second;
	}

	if (!wrapper) {
		std::vector<std::shared_ptr<std::string>> font_data;

		std::vector<std::string> paths;
		paths.push_back(!bold ? "Play-Regular.ttf" : "Play-Bold.ttf");
		if (lang_str == "zh_Hans")
			paths.push_back(!bold ? "NotoSansSC-Regular.otf" : "NotoSansSC-Bold.otf");
		if (lang_str == "zh_Hant")
			paths.push_back(!bold ? "NotoSansTC-Regular.otf" : "NotoSansTC-Bold.otf");

		for (const std::string &path : paths) {
			auto it = font_files.find(path);
			if (it != font_files.end()) {
				font_data.push_back(it->second);
				continue;
			}

			std::optional<std::string> data = read_file("fonts/" + path);
			if (!data)
				continue;

			auto s = std::make_shared<std::string>(std::move(*data));
			font_files.insert_or_assign(path, s);
			font_data.push_back(s);
		}

		if (font_data.empty())
			return nullptr;

		wrapper = std::make_shared<SimrailFontWrapper>();
		wrapper->font_data = std::move(font_data);
		wrapper->size = coarse_size;
		wrapper->current = nullptr;
		wrapper->pending = nullptr;
		wrapper->ranges_builder.AddChar('?');

		loaded_fonts.insert_or_assign({ coarse_size, bold, lang_str }, wrapper);
	}

	return std::make_unique<SimrailFont>(std::move(wrapper), ascent, *this);
}

std::unique_ptr<SimrailUiPlatform::Image> SimrailUiPlatform::make_text_image(const std::string_view text, const Font &base, Color c) {
	const SimrailFont &font = dynamic_cast<const SimrailFont&>(base);

	auto image = std::make_unique<SimrailImage>();
	image->font.emplace(font.font, font.ascent, font.platform);
	image->text = text;
	image->text_size = font.calc_size(text);
	image->color = IM_COL32(c.R, c.G, c.B, 255);

	return image;
}

std::unique_ptr<SimrailUiPlatform::Image> SimrailUiPlatform::make_wrapped_text_image(const std::string_view text, const Font &base, int align, Color c) {
	return make_text_image(text, base, c);
}

namespace api {
	IMPORT_FUNC("simrail_ui_v1", "sound_create") uint32_t sound_create(const char* path, size_t len);
	IMPORT_FUNC("simrail_ui_v1", "sound_destroy") void sound_destroy(uint32_t handle);
	IMPORT_FUNC("simrail_ui_v1", "source_create") uint32_t source_create(uint32_t sound, int32_t looping);
	IMPORT_FUNC("simrail_ui_v1", "source_destroy") void source_destroy(uint32_t handle, int32_t keep_playing);
	IMPORT_FUNC("simrail_ui_v1", "set_volume") void set_volume(float volume);
}

SimrailUiPlatform::SimrailSoundData::SimrailSoundData(uint32_t h) : handle(h) {

}

SimrailUiPlatform::SimrailSoundData::~SimrailSoundData() {
	api::sound_destroy(handle);
}

uint32_t SimrailUiPlatform::SimrailSoundData::get() const {
	return handle;
}

SimrailUiPlatform::SimrailSoundSource::SimrailSoundSource(uint32_t h) : handle(h) {

}

SimrailUiPlatform::SimrailSoundSource::~SimrailSoundSource() {
	if (handle)
		api::source_destroy(*handle, 0);
}

void SimrailUiPlatform::SimrailSoundSource::detach() {
	if (handle) {
		api::source_destroy(*handle, 1);
		handle.reset();
	}
}

void SimrailUiPlatform::set_volume(int vol) {
	api::set_volume(vol / 100.0f);
	last_volume = vol;
}

int SimrailUiPlatform::get_volume() {
	return last_volume;
}

std::unique_ptr<SimrailUiPlatform::SoundData> SimrailUiPlatform::load_sound(const std::string_view path) {
	uint32_t handle = api::sound_create(path.data(), path.size());
	if (handle == 0)
		return nullptr;

	return std::make_unique<SimrailSoundData>(handle);
}

std::unique_ptr<SimrailUiPlatform::SoundData> SimrailUiPlatform::load_sound(const std::vector<std::pair<int, int>> &melody) {
	std::string desc = "=";
	for (const auto &tone : melody)
		desc += "," + std::to_string(tone.first) + "/" + std::to_string(tone.second);

	return load_sound(desc);
}

std::unique_ptr<SimrailUiPlatform::SoundSource> SimrailUiPlatform::play_sound(const SoundData &base, bool looping) {
	const SimrailSoundData &data = dynamic_cast<const SimrailSoundData&>(base);

	uint32_t handle = api::source_create(data.get(), looping ? 1 : 0);
	return std::make_unique<SimrailSoundSource>(handle);
}

void SimrailUiPlatform::set_brightness(int br) {
	api::set_brightness(br / 100.0f);
	last_brightness = br;
}

int SimrailUiPlatform::get_brightness() {
	return last_brightness;
}

Promise<UiPlatform::InputEvent> SimrailUiPlatform::on_input_event() {
	auto pair = PromiseFactory::create<UiPlatform::InputEvent>();
	api::on_input_event((void*)&callback_fulfill_input_event, (void*)&callback_cancel_void, new Fulfiller(std::move(pair.second)));
	return std::move(pair.first);
}
