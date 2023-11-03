 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"
#include "imgui.h"
#include <map>

class SimrailBasePlatform final : public BasePlatform {
private:
	class SimrailBusSocket final : public BusSocket {
	private:
		uint32_t handle;
	public:
		SimrailBusSocket(uint32_t handle);
		~SimrailBusSocket() override;
		void broadcast(const std::string_view data) override;
		void broadcast(uint32_t tid, const std::string_view data) override;
		void send_to(uint32_t uid, const std::string_view data) override;
		PlatformUtil::Promise<BasePlatform::BusSocket::ReceiveResult> receive() override;
	};

	std::vector<std::unique_ptr<PlatformUtil::TypeErasedFulfiller>> event_list;

public:
	SimrailBasePlatform();

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

class SimrailUiPlatform final : public UiPlatform {
private:
	friend class SimrailFont;

	struct stbi_deleter { void operator()(uint8_t *ptr); };

	struct SimrailFontWrapper {
		std::vector<std::shared_ptr<std::string>> font_data;
		float size;
		ImFontGlyphRangesBuilder ranges_builder;
		ImFont *current;
		ImFont *pending;
	};

	struct AtlasRect {
		int x, y;
		float uv0x, uv0y, uv1x, uv1y;
	};

	struct SimrailImageWrapper {
		std::unique_ptr<uint8_t[], stbi_deleter> data;
		int width;
		int height;

		std::optional<AtlasRect> current;
		std::optional<AtlasRect> pending;
	};

	SimrailBasePlatform base;

	ImU32 current_color;
	ImDrawList *drawlist;

	std::map<std::string, std::shared_ptr<SimrailImageWrapper>, std::less<>> loaded_images;
	std::map<std::tuple<float, bool, std::string>, std::shared_ptr<SimrailFontWrapper>> loaded_fonts;
	std::map<std::string, std::shared_ptr<std::string>> font_files;
	uint32_t atlas_id;
	std::unique_ptr<ImFontAtlas> current_atlas;
	std::unique_ptr<ImFontAtlas> pending_atlas;
	void build_atlas();
	void upload_atlas();

	std::vector<InputEvent> pending_events;
	PlatformUtil::Promise<InputEvent> input_promise;
	void handle_event(InputEvent ev);

	int last_volume;

public:
	class SimrailFont : public Font
	{
	public:
		SimrailUiPlatform &platform;
		std::shared_ptr<SimrailFontWrapper> font;
		float size;

	public:
		SimrailFont(std::shared_ptr<SimrailFontWrapper> wrapper, float s, SimrailUiPlatform &p);
		float ascent() const override;
		std::pair<float, float> calc_size(const std::string_view str) const override;
	};

	class SimrailImage : public Image
	{
	public:
		SimrailImage() = default;
		float width() const override;
		float height() const override;

		std::optional<SimrailFont> font;
		mutable std::optional<std::pair<float, float>> text_size;
		std::string text;
		unsigned int color;

		std::shared_ptr<SimrailImageWrapper> image;
	};

	class SimrailSoundData : public SoundData
	{
		uint32_t handle;

	public:
		SimrailSoundData(uint32_t h);
		~SimrailSoundData();
		uint32_t get() const;
	};

	class SimrailSoundSource : public SoundSource
	{
		std::optional<uint32_t> handle;

	public:
		SimrailSoundSource(uint32_t h);
		~SimrailSoundSource();
		void detach() override;
	};

	SimrailUiPlatform(float virtual_w, float virtual_h);
	~SimrailUiPlatform() override;

	int64_t get_timer() override { return base.get_timer(); };
	int64_t get_timestamp() override { return base.get_timestamp(); };
	DateTime get_local_time() override { return base.get_local_time(); };

	std::unique_ptr<BusSocket> open_socket(const std::string_view channel, uint32_t tid) override { return base.open_socket(channel, tid); };
	std::optional<std::string> read_file(const std::string_view path) override { return base.read_file(path); };
	bool write_file(const std::string_view path, const std::string_view contents) override { return base.write_file(path, contents); };
	void debug_print(const std::string_view msg) override { base.debug_print(msg); };

	PlatformUtil::Promise<void> delay(int ms) override { return base.delay(ms); };
	PlatformUtil::Promise<void> on_quit_request() override { return base.on_quit_request(); };
	PlatformUtil::Promise<void> on_quit() override { return base.on_quit(); };

	void quit() override { base.quit(); };

	void set_color(Color c) override;
	void draw_line(float x1, float y1, float x2, float y2) override;
	void draw_rect(float x, float y, float w, float h) override;
	void draw_rect_filled(float x, float y, float w, float h) override;
	void draw_image(const Image &img, float x, float y, float w, float h) override;
	void draw_circle_filled(float x, float y, float r) override;
	void draw_arc_filled(float x, float y, float r_min, float r_max, float a_min, float a_max) override;
	void draw_convex_polygon_filled(const std::vector<std::pair<float, float>> &poly) override;
	PlatformUtil::Promise<void> on_present_request() override;
	void present() override;
	std::unique_ptr<Image> load_image(const std::string_view path) override;
	std::unique_ptr<Font> load_font(float size, bool bold, const std::string_view lang) override;
	std::unique_ptr<Image> make_text_image(const std::string_view text, const Font &font, Color c) override;
	std::unique_ptr<Image> make_wrapped_text_image(const std::string_view text, const Font &font, int align, Color c) override;

	void set_volume(int vol) override;
	int get_volume() override;
	std::unique_ptr<SoundData> load_sound(const std::string_view path) override;
	std::unique_ptr<SoundData> load_sound(const std::vector<std::pair<int, int>> &melody) override;
	std::unique_ptr<SoundSource> play_sound(const SoundData &snd, bool looping) override;

	void set_brightness(int br) override;
	int get_brightness() override;

	PlatformUtil::Promise<InputEvent> on_input_event() override;
};
