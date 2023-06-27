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
#include "console_fd_poller.h"

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;
struct _TTF_Font;
typedef struct _TTF_Font TTF_Font;

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
	std::string load_path;
	Color current_color;
	int audio_samplerate;
	int audio_device;
	int audio_volume;
	std::map<std::pair<float, bool>, std::shared_ptr<SdlFontWrapper>> loaded_fonts;
	float s, ox, oy;
	std::multimap<int, PlatformUtil::Fulfiller<void>> timer_queue;
	PlatformUtil::FulfillerList<void> on_close_list;
	PlatformUtil::FulfillerList<void> on_quit_list;
	PlatformUtil::FulfillerList<void> on_present_list;
	PlatformUtil::FulfillerList<InputEvent> on_input_list;
	bool running;
	std::map<std::string, std::string, std::less<>> ini_items;
	void load_config();
	std::string get_config(const std::string_view key);
	BusSocketImpl bus_socket_impl;
	LibcTimeImpl libc_time_impl;
	FstreamFileImpl fstream_file_impl;
	std::vector<std::unique_ptr<PlatformUtil::TypeErasedFulfiller>> event_list;

	std::vector<std::shared_ptr<PlaybackState>> playback_list;
	static void mixer_func_proxy(void *ptr, unsigned char *stream, int len);
	void mixer_func(int16_t *buffer, size_t len);
	bool poll_sdl();

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
		float width() const override;
		float height() const override;
	};

	class SdlFont final : public Font
	{
	private:
		std::shared_ptr<SdlFontWrapper> font;
		float scale;
	public:
		SdlFont(std::shared_ptr<SdlFontWrapper> wrapper, float scale);
		TTF_Font* get() const;
		float ascent() const override;
		std::pair<float, float> calc_size(const std::string_view str) const override;
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

	SdlPlatform(float virtual_w, float virtual_h);
	void event_loop();

	~SdlPlatform() override;

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

	void set_color(Color c) override;
	void draw_line(float x1, float y1, float x2, float y2) override;
	void draw_rect(float x, float y, float w, float h) override;
	void draw_rect_filled(float x, float y, float w, float h) override;
	void draw_image(const Image &img, float x, float y, float w, float h) override;
	void draw_circle_filled(float x, float y, float rad) override;
	void draw_polygon_filled(const std::vector<std::pair<float, float>> &poly) override;
	void clear() override;
	PlatformUtil::Promise<void> present() override;
	std::unique_ptr<Image> load_image(const std::string_view path) override;
	std::unique_ptr<Font> load_font(float size, bool bold) override;
	std::unique_ptr<Image> make_text_image(const std::string_view text, const Font &font, Color c) override;
	std::unique_ptr<Image> make_wrapped_text_image(const std::string_view text, const Font &font, int align, Color c) override;

	void set_volume(int vol) override;
	std::unique_ptr<SoundData> load_sound(const std::string_view path) override;
	std::unique_ptr<SoundData> load_sound(const std::vector<std::pair<int, int>> &melody) override;
	std::unique_ptr<SoundSource> play_sound(const SoundData &snd, bool looping) override;

	void set_brightness(int br) override;

	PlatformUtil::Promise<InputEvent> on_input_event() override;
};
