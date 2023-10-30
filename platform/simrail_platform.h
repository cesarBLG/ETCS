 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"
#include "imgui.h"

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
	SimrailBasePlatform base;

	unsigned int current_color;
	ImDrawList *drawlist;

	std::vector<InputEvent> pending_events;
	PlatformUtil::Promise<InputEvent> input_promise;
	void handle_event(InputEvent ev);

public:
	class SimrailImage : public Image
	{
	public:
		SimrailImage() = default;
		float width() const override { return 10.0f; };
		float height() const override { return 10.0f; };
	};

	class SimrailFont : public Font
	{
	public:
		SimrailFont() = default;
		float ascent() const override { return 3.0f; }
		std::pair<float, float> calc_size(const std::string_view str) const override { return std::make_pair(10.0f, 10.0f); }
	};

	class SimrailSoundData : public SoundData
	{
	public:
		SimrailSoundData() = default;
	};

	class SimrailSoundSource : public SoundSource
	{
	public:
		SimrailSoundSource() = default;
		void detach() override {};
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
	void draw_circle_filled(float x, float y, float rad) override;
	void draw_polygon_filled(const std::vector<std::pair<float, float>> &poly) override;
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
