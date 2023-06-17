 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include <atomic>
#include "platform.h"

struct SDL_Renderer;
struct SDL_Texture;
struct _TTF_Font;
typedef struct _TTF_Font TTF_Font;

class SdlPlatform : public Platform {
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
	std::map<std::pair<float, bool>, std::shared_ptr<SdlFontWrapper>> loaded_fonts;

	std::vector<std::shared_ptr<PlaybackState>> playback_list;
	static void mixer_func_proxy(void *ptr, unsigned char *stream, int len);
	void mixer_func(int16_t *buffer, size_t len);

public:
	class SdlImage : public Image
	{
	private:
		SDL_Texture* tex;
		int w, h;
	public:
		SdlImage(SDL_Texture *tex, int w, int h);
		SDL_Texture* get() const;
		virtual ~SdlImage() override;
		virtual int width() const override;
		virtual int height() const override;
	};

	class SdlFont : public Font
	{
	private:
		std::shared_ptr<SdlFontWrapper> font;
	public:
		SdlFont(std::shared_ptr<SdlFontWrapper> wrapper);
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

	SdlPlatform(SDL_Renderer *r);
	virtual ~SdlPlatform() override;

	virtual void set_color(Color c) override;
	virtual void draw_line(int x1, int y1, int x2, int y2) override;
	virtual void draw_rect(int x, int y, int w, int h) override;
	virtual void draw_rect_filled(int x, int y, int w, int h) override;
	virtual void draw_image(const Image &img, int x, int y, int w, int h) override;
	virtual void draw_circle_filled(int x, int y, int rad) override;
	virtual void draw_polygon_filled(const short int *vx, const short int *vy, size_t n) override;
	virtual void clear() override;
	virtual std::unique_ptr<Image> load_image(const std::string &path) override;
	virtual std::unique_ptr<Font> load_font(float size, bool bold) override;
	virtual std::unique_ptr<Image> make_text_image(const std::string &text, const Font &font, Color c) override;
	virtual std::unique_ptr<Image> make_wrapped_text_image(const std::string &text, const Font &font, int align, Color c) override;

	virtual std::unique_ptr<SoundData> load_sound(const std::string &path) override;
	virtual std::unique_ptr<SoundData> load_sound(const std::vector<std::pair<int, int>> &melody) override;
	virtual std::unique_ptr<SoundSource> play_sound(const SoundData &snd, bool looping) override;
};
