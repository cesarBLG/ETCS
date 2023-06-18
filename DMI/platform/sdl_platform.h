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

struct SDL_Renderer;
struct SDL_Texture;
struct _TTF_Font;
typedef struct _TTF_Font TTF_Font;

class SdlPlatform : public UiPlatform {
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

	SDL_Renderer *sdlrend;
	std::string load_path;
	Color current_color;
	int audio_samplerate;
	int audio_device;
	int audio_volume;
	std::map<std::pair<float, bool>, std::shared_ptr<SdlFontWrapper>> loaded_fonts;
	float s, ox, oy;
	std::map<int, PlatformUtil::Fulfiller<void>> timer_queue;
	PlatformUtil::FulfillerList<void> on_close_list;
	PlatformUtil::FulfillerList<void> on_quit_list;
	PlatformUtil::FulfillerList<InputEvent> on_input_list;
	PlatformUtil::FulfillerList<void> on_present_list;
	size_t present_count;
	bool running;

	std::vector<std::shared_ptr<PlaybackState>> playback_list;
	static void mixer_func_proxy(void *ptr, unsigned char *stream, int len);
	void mixer_func(int16_t *buffer, size_t len);

public:
	class SdlImage : public Image
	{
	private:
		SDL_Texture* tex;
		float w, h, scale;

	public:
		SdlImage(SDL_Texture *tex, float w, float h, float s);
		SDL_Texture* get() const;
		virtual ~SdlImage() override;
		virtual float width() const override;
		virtual float height() const override;
	};

	class SdlFont : public Font
	{
	private:
		std::shared_ptr<SdlFontWrapper> font;
		float scale;
	public:
		SdlFont(std::shared_ptr<SdlFontWrapper> wrapper, float scale);
		TTF_Font* get() const;
		virtual float ascent() const override;
		virtual std::pair<float, float> calc_size(const std::string &str) const override;
	};

	class SdlSoundData : public SoundData
	{
	private:
		std::shared_ptr<SdlSoundDataWrapper> data;
	public:
		SdlSoundData(const std::shared_ptr<SdlSoundDataWrapper> &wrap);
		const std::shared_ptr<SdlSoundDataWrapper>& get() const;
	};

	class SdlSoundSource : public SoundSource
	{
	private:
		std::weak_ptr<PlaybackState> state;
	public:
		SdlSoundSource(const std::shared_ptr<PlaybackState> &s);
		virtual ~SdlSoundSource() override;
		virtual void detach() override;
	};

	SdlPlatform(SDL_Renderer *r, float s, float ox, float oy);
	virtual ~SdlPlatform() override;

	virtual int64_t get_timer() override;
	virtual int64_t get_timestamp() override;
	virtual DateTime get_local_time() override;

	virtual std::string read_file(const std::string &path) override;
	virtual void debug_print(const std::string &msg) override;

	virtual PlatformUtil::Promise<void> delay(int ms) override;
	virtual PlatformUtil::Promise<void> on_quit_request() override;
	virtual PlatformUtil::Promise<void> on_quit() override;

	virtual void event_loop() override;
	virtual void quit() override;

	virtual void set_color(Color c) override;
	virtual void draw_line(float x1, float y1, float x2, float y2) override;
	virtual void draw_rect(float x, float y, float w, float h) override;
	virtual void draw_rect_filled(float x, float y, float w, float h) override;
	virtual void draw_image(const Image &img, float x, float y, float w, float h) override;
	virtual void draw_circle_filled(float x, float y, float rad) override;
	virtual void draw_polygon_filled(const std::vector<std::pair<float, float>> &poly) override;
	virtual void clear() override;
	virtual PlatformUtil::Promise<void> present() override;
	virtual std::unique_ptr<Image> load_image(const std::string &path) override;
	virtual std::unique_ptr<Font> load_font(float size, bool bold) override;
	virtual std::unique_ptr<Image> make_text_image(const std::string &text, const Font &font, Color c) override;
	virtual std::unique_ptr<Image> make_wrapped_text_image(const std::string &text, const Font &font, int align, Color c) override;

	virtual void set_volume(int vol) override;
	virtual std::unique_ptr<SoundData> load_sound(const std::string &path) override;
	virtual std::unique_ptr<SoundData> load_sound(const std::vector<std::pair<int, int>> &melody) override;
	virtual std::unique_ptr<SoundSource> play_sound(const SoundData &snd, bool looping) override;

	virtual void set_brightness(int br) override;

	virtual PlatformUtil::Promise<InputEvent> on_input_event() override;
};
