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
#include <SDL.h>
#include <SDL_ttf.h>
#include "platform.h"
#include "bus_socket_impl.h"
#include "libc_time_impl.h"
#include "fstream_file_impl.h"
#include "console_fd_poller.h"

class SdlPlatform final : public UiPlatform {
private:
	struct SdlFontWrapper
	{
		TTF_Font* font;

		SdlFontWrapper(TTF_Font *f);
		~SdlFontWrapper();
	};

	struct SdlSoundDataWrapper
	{
		std::unique_ptr<int16_t[]> buffer;
		size_t samples;

		SdlSoundDataWrapper(std::unique_ptr<int16_t[]> &&b, size_t s);
	};

	struct PlaybackState {
		std::shared_ptr<SdlSoundDataWrapper> data;
		size_t position;
		bool looping;
		std::atomic<bool> stop;
	};

	ConsoleFdPoller poller;

	SDL_Renderer *sdlrend;
	SDL_Window *sdlwindow;
	std::string assets_dir;
	std::string config_dir;
	std::string storage_dir;
	Color current_color;
	int audio_samplerate;
	int audio_device;
	int audio_volume;
	std::map<std::tuple<float, bool, std::string>, std::shared_ptr<SdlFontWrapper>> loaded_fonts;
	float s, ox, oy;
	float dpiscale;
	float virtual_w, virtual_h;
	bool rotate;
	int wx, wy;
	bool touch;
	std::multimap<int64_t, PlatformUtil::Fulfiller<void>> timer_queue;
	PlatformUtil::FulfillerList<void> on_close_list;
	PlatformUtil::FulfillerList<void> on_quit_list;
	PlatformUtil::FulfillerList<void> on_present_list;
	PlatformUtil::FulfillerList<InputEvent> on_input_list;
	int present_count;
	bool running;
	std::map<std::string, std::string, std::less<>> ini_items;
	void load_config(const std::vector<std::string>& args);
	std::string get_config(const std::string_view key);
	std::string get_config(const std::string_view key, const std::string_view def);
	BusSocketImpl bus_socket_impl;
	LibcTimeImpl libc_time_impl;
	FstreamFileImpl fstream_file_impl;
	std::vector<std::unique_ptr<PlatformUtil::TypeErasedFulfiller>> event_list;

	std::vector<std::shared_ptr<PlaybackState>> playback_list;
	static void mixer_func_proxy(void *ptr, unsigned char *stream, int len);
	void mixer_func(int16_t *buffer, size_t len);
	bool poll_sdl();
	void draw_polygon_filled(const std::vector<std::pair<float, float>> &poly);
	void calc_scale();

public:
	class SdlImage final : public Image
	{
	private:
		SDL_Texture* tex;
		float w, h, scale;

	public:
		SdlImage(SDL_Texture *tex, float w, float h, float s);
		SDL_Texture* get() const;
		~SdlImage() override;
		std::pair<float, float> size() const override;
	};

	class SdlFont final : public Font
	{
	private:
		std::shared_ptr<SdlFontWrapper> font;
		float scale;
	public:
		SdlFont(std::shared_ptr<SdlFontWrapper> wrapper, float scale);
		TTF_Font* get() const;
		std::pair<float, float> calc_size(const std::string_view str, float wrap_width = 0.0f) const override;
		size_t calc_wrap_point(const std::string_view str, float wrap_width) const override;
	};

	class SdlSoundData final : public SoundData
	{
	private:
		std::shared_ptr<SdlSoundDataWrapper> data;
	public:
		SdlSoundData(const std::shared_ptr<SdlSoundDataWrapper> &wrap);
		const std::shared_ptr<SdlSoundDataWrapper>& get() const;
	};

	class SdlSoundSource final : public SoundSource
	{
	private:
		std::weak_ptr<PlaybackState> state;
	public:
		SdlSoundSource(const std::shared_ptr<PlaybackState> &s);
		~SdlSoundSource() override;
		void detach() override;
	};

	SdlPlatform(float virtual_w, float virtual_h, const std::vector<std::string> &args);
	void event_loop();

	~SdlPlatform() override;

	int64_t get_timer() override;

	std::unique_ptr<BusSocket> open_socket(const std::string_view channel, uint32_t tid) override;
	std::optional<std::string> read_file(const std::string_view path, FileType file_type=ETCS_ASSET_FILE) override;
	bool write_file(const std::string_view path, const std::string_view contents) override;
	void debug_print(const std::string_view msg) override;

	PlatformUtil::Promise<void> delay(int ms) override;
	PlatformUtil::Promise<void> on_quit_request() override;
	PlatformUtil::Promise<void> on_quit() override;

	void quit() override;

	void set_color(Color c) override;
	void draw_line(float x1, float y1, float x2, float y2) override;
	void draw_rect(float x, float y, float w, float h) override;
	void draw_rect_filled(float x, float y, float w, float h) override;
	void draw_image(const Image &img, float x, float y) override;
	void draw_arc_filled(float x, float y, float r_min, float r_max, float a_min, float a_max) override;
	void draw_circle_filled(float x, float y, float r) override;
	void draw_convex_polygon_filled(const std::vector<std::pair<float, float>> &poly) override;
	PlatformUtil::Promise<void> on_present_request() override;
	void present() override;
	std::unique_ptr<Image> load_image(const std::string_view path) override;
	std::unique_ptr<Font> load_font(float size, bool bold, const std::string_view lang) override;
	std::unique_ptr<Image> make_text_image(const std::string_view text, const Font &font, Color c) override;
	std::unique_ptr<Image> make_wrapped_text_image(const std::string_view text, const Font &font, float width, int align, Color c) override;

	void set_volume(int vol) override;
	int get_volume() override;
	std::unique_ptr<SoundData> load_sound(const std::string_view path) override;
	std::unique_ptr<SoundData> load_sound(const std::vector<std::pair<int, int>> &melody) override;
	std::unique_ptr<SoundSource> play_sound(const SoundData &snd, bool looping) override;

	void set_brightness(int br) override;
	int get_brightness() override;

	PlatformUtil::Promise<InputEvent> on_input_event() override;
};
