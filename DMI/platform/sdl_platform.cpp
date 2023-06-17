 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "sdl_platform.h"
#include "../graphics/drawing.h"
#include "../graphics/sdl/gfx_primitives.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <ctime>

SdlPlatform::SdlPlatform(SDL_Renderer *r, float s, float ox, float oy) : sdlrend(r), s(s), ox(ox), oy(oy) {
#ifdef __ANDROID__
	extern std::string filesDir;
	load_path = filesDir + "/";
#endif
	SDL_AudioSpec desired = {};
	SDL_AudioSpec obtained;
	desired.freq = 44100;
	desired.format = AUDIO_S16;
	desired.channels = 1;
	desired.samples = 2048;
	desired.callback = &mixer_func_proxy;
	desired.userdata = this;
	audio_device = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	audio_samplerate = obtained.freq;
	SDL_PauseAudioDevice(audio_device, 0);
	TTF_Init();
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, s == std::floor(s) ? "0" : "1");
}

SdlPlatform::~SdlPlatform() {
	SDL_CloseAudioDevice(audio_device);
}

void SdlPlatform::set_color(Color c) {
	SDL_SetRenderDrawColor(sdlrend, c.R, c.G, c.B, 255);
	current_color = c;
}

void SdlPlatform::draw_line(float x1, float y1, float x2, float y2) {
	SDL_RenderDrawLineF(sdlrend, x1 * s + ox, y1 * s + oy, x2 * s + ox, y2 * s + oy);
}

void SdlPlatform::draw_rect(float x, float y, float w, float h) {
	SDL_FRect rect { x * s + ox, y * s + oy, w * s, h * s };
	SDL_RenderDrawRectF(sdlrend, &rect);
}

void SdlPlatform::draw_rect_filled(float x, float y, float w, float h) {
	SDL_FRect rect { x * s + ox, y * s + oy, w * s, h * s };
	SDL_RenderFillRectF(sdlrend, &rect);
}

void SdlPlatform::draw_image(const Image &base, float x, float y, float w, float h) {
	const SdlImage &img = dynamic_cast<const SdlImage&>(base);
	if (s > 0.0f) {
		SDL_FRect rect { std::floor(x * s + ox), std::floor(y * s + oy), w * s, h * s };
		SDL_RenderCopyF(sdlrend, img.get(), nullptr, &rect);
	} else {
		SDL_FRect rect { std::floor((x + w) * s + ox), std::floor((y + h) * s + oy), w * -s, h * -s };
		SDL_RenderCopyExF(sdlrend, img.get(), nullptr, &rect, 180.0, nullptr, SDL_FLIP_NONE);
	}
}

void SdlPlatform::draw_circle_filled(float x, float y, float rad) {
	Color c = current_color;
	filledCircleRGBA(sdlrend, x * s + ox, y * s + oy, rad * s, c.R, c.G, c.B, 255);
	aacircleRGBA(sdlrend, x * s + ox, y * s + oy, rad * s, c.R, c.G, c.B, 255);
}

void SdlPlatform::draw_polygon_filled(const std::vector<std::pair<float, float>> &poly) {
	std::vector<int16_t> sx, sy;
	sx.reserve(poly.size());
	sy.reserve(poly.size());
	for (const std::pair<float, float> &v : poly) {
		sx.push_back(v.first * s + ox);
		sy.push_back(v.second * s + oy);
	}
	Color c = current_color;
	filledPolygonRGBA(sdlrend, sx.data(), sy.data(), poly.size(), c.R, c.G, c.B, 255);
	aapolygonRGBA(sdlrend, sx.data(), sy.data(), poly.size(), c.R, c.G, c.B, 255);
}

void SdlPlatform::clear() {
	SDL_RenderClear(sdlrend);
}

std::unique_ptr<Platform::Image> SdlPlatform::load_image(const std::string &p) {
	std::string path = load_path + p;
	SDL_Surface *surf = SDL_LoadBMP(path.c_str());
	if (surf == nullptr) {
		printf("Error loading BMP %s. SDL Error: %s\n", path.c_str(), SDL_GetError());
		return nullptr;
	}
	SDL_Texture *tex = SDL_CreateTextureFromSurface(sdlrend, surf);
	if (tex == nullptr) {
		printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		SDL_FreeSurface(surf);
		return nullptr;
	}
	std::unique_ptr<SdlImage> img = std::make_unique<SdlImage>(tex, surf->w, surf->h, 1.0f);
	SDL_FreeSurface(surf);
	return img;
}

std::unique_ptr<Platform::Font> SdlPlatform::load_font(float size, bool bold) {
	auto it = loaded_fonts.find({size, bold});
	std::shared_ptr<SdlFontWrapper> wrapper;
	if (it != loaded_fonts.end())
		wrapper = it->second;

	float scale = std::abs(s);

	if (!wrapper) {
#if SIMRAIL
		std::string path = load_path + (!bold ? "fonts/verdana.ttf" : "fonts/verdanab.ttf");
		TTF_Font* font = TTF_OpenFont(path.c_str(), size * 1.25 * scale);
#else
		std::string path = load_path + (!bold ? "fonts/swiss.ttf" : "fonts/swissb.ttf");
		TTF_Font* font = TTF_OpenFont(path.c_str(), size * 1.4 * scale);
#endif
		if (font == nullptr)
			return nullptr;
		wrapper = std::make_shared<SdlFontWrapper>(font);
		loaded_fonts.insert_or_assign({size, bold}, wrapper);
	}

	return std::make_unique<SdlFont>(wrapper, scale);
}

std::unique_ptr<Platform::Image> SdlPlatform::make_text_image(const std::string &text, const Font &base, Color c) {
	if (text.empty())
		return nullptr;

	const SdlFont &font = dynamic_cast<const SdlFont&>(base);

	SDL_Color color = { c.R, c.G, c.B };
	SDL_Surface *surf = TTF_RenderUTF8_Blended(font.get(), text.c_str(), color);
	if (surf == nullptr) {
		printf("TTF render failed\n");
		return nullptr;
	}

	SDL_Texture *tex = SDL_CreateTextureFromSurface(sdlrend, surf);
	if (tex == nullptr) {
		printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		SDL_FreeSurface(surf);
		return nullptr;
	}

	std::unique_ptr<SdlImage> img = std::make_unique<SdlImage>(tex, surf->w, surf->h, std::abs(s));
	SDL_FreeSurface(surf);
	return img;
}

std::unique_ptr<Platform::Image> SdlPlatform::make_wrapped_text_image(const std::string &text, const Font &base, int align, Color c) {
	if (text.empty())
		return nullptr;

	const SdlFont &font = dynamic_cast<const SdlFont&>(base);

	TTF_SetFontWrappedAlign(font.get(), align == CENTER ? TTF_WRAPPED_ALIGN_CENTER : (align == RIGHT ? TTF_WRAPPED_ALIGN_RIGHT : TTF_WRAPPED_ALIGN_LEFT));

	//if (aspect & 2) TTF_SetFontStyle(font, TTF_STYLE_UNDERLINE);
	SDL_Color color = { c.R, c.G, c.B };
	SDL_Surface *surf = TTF_RenderUTF8_Blended_Wrapped(font.get(), text.c_str(), color, 0);
	if (surf == nullptr) {
		printf("TTF render failed\n");
		return nullptr;
	}

	SDL_Texture *tex = SDL_CreateTextureFromSurface(sdlrend, surf);
	if (tex == nullptr) {
		printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		SDL_FreeSurface(surf);
		return nullptr;
	}

	std::unique_ptr<SdlImage> img = std::make_unique<SdlImage>(tex, surf->w, surf->h, std::abs(s));
	SDL_FreeSurface(surf);
	return img;
}

SdlPlatform::SdlImage::SdlImage(SDL_Texture *tex, float w, float h, float s) : tex(tex), w(w), h(h), scale(s) {

}

SdlPlatform::SdlImage::~SdlImage() {
	SDL_DestroyTexture(tex);
}

SDL_Texture* SdlPlatform::SdlImage::get() const {
	return tex;
}

float SdlPlatform::SdlImage::width() const {
	return w / scale;
}

float SdlPlatform::SdlImage::height() const {
	return h / scale;
}

SdlPlatform::SdlFontWrapper::SdlFontWrapper(TTF_Font *f) : font(f) {

}

SdlPlatform::SdlFontWrapper::~SdlFontWrapper() {
	TTF_CloseFont(font);
}

SdlPlatform::SdlFont::SdlFont(std::shared_ptr<SdlFontWrapper> wrapper, float s) : font(wrapper), scale(s) {

}

float SdlPlatform::SdlFont::ascent() const {
	return TTF_FontAscent(font->font) / scale;
}

std::pair<float, float> SdlPlatform::SdlFont::calc_size(const std::string &str) const {
	int w, h;
	TTF_SizeUTF8(font->font, str.c_str(), &w, &h);
	return std::make_pair(w / scale, h / scale);
}

TTF_Font* SdlPlatform::SdlFont::get() const {
	return font->font;
}

SdlPlatform::SdlSoundDataWrapper::SdlSoundDataWrapper(std::unique_ptr<int16_t[]> &&b, size_t s) : buffer(std::move(b)), samples(s) {

}

SdlPlatform::SdlSoundData::SdlSoundData(const std::shared_ptr<SdlSoundDataWrapper> &wrap) : data(wrap) {

}

const std::shared_ptr<SdlPlatform::SdlSoundDataWrapper>& SdlPlatform::SdlSoundData::get() const {
	return data;
}

void SdlPlatform::mixer_func_proxy(void *ptr, unsigned char *stream, int len) {
	((SdlPlatform*)ptr)->mixer_func((int16_t*)stream, len / 2);
}

void SdlPlatform::mixer_func(int16_t *buffer, size_t len) {
	memset(buffer, 0, len * 2);

	playback_list.erase(std::remove_if(playback_list.begin(), playback_list.end(), [=](const std::shared_ptr<PlaybackState> &state)
	{
		if (state->stop)
			return true;

		size_t i = 0;
		while (i < len) {
			buffer[i++] = std::clamp(buffer[i] + state->data->buffer[state->position++], INT16_MIN, INT16_MAX);

			if (state->position == state->data->samples) {
				if (state->looping)
					state->position = 0;
				else
					return true;
			}
		}

		return false;
	}), playback_list.end());
}

SdlPlatform::SdlSoundSource::SdlSoundSource(const std::shared_ptr<PlaybackState> &s) : state(s) {

}

SdlPlatform::SdlSoundSource::~SdlSoundSource() {
	std::shared_ptr<PlaybackState> locked = state.lock();
	if (locked)
		locked->stop = true;
}

void SdlPlatform::SdlSoundSource::detach() {
	state.reset();
}

std::unique_ptr<Platform::SoundData> SdlPlatform::load_sound(const std::string &path) {
	std::string file = load_path + "sound/" + path + ".wav";

	SDL_AudioSpec spec;
	uint8_t* buffer;
	uint32_t len;
	if (!SDL_LoadWAV(file.c_str(), &spec, &buffer, &len))
		return nullptr;

	SDL_AudioCVT cvt;
	int ret = SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq, AUDIO_S16, 1, audio_samplerate);
	if (ret < 0)
		return nullptr;

	std::unique_ptr<int16_t[]> buf = std::make_unique<int16_t[]>(len * cvt.len_mult / 2);
	memcpy(buf.get(), buffer, len);
	SDL_FreeWAV(buffer);

	cvt.buf = (Uint8*)buf.get();
	cvt.len = len;

	if (SDL_ConvertAudio(&cvt))
		return nullptr;

	return std::make_unique<SdlSoundData>(std::make_shared<SdlSoundDataWrapper>(std::move(buf), cvt.len * cvt.len_ratio / 2));
}

std::unique_ptr<Platform::SoundData> SdlPlatform::load_sound(const std::vector<std::pair<int, int>> &melody) {
	std::vector<int16_t> buffer;
	for (std::pair<int, int> element : melody)
	{
		int samples = (float)audio_samplerate / 1000 * element.second;
		if (element.first > 0) {
			float x = (float)element.first / audio_samplerate * 2.0f * M_PI;
			for (int i = 0; i < samples; i++)
				buffer.push_back(std::cos(x * i) * INT16_MAX);
		} else {
			for (int i = 0; i < samples; i++)
				buffer.push_back(0);
		}
	}

	std::unique_ptr<int16_t[]> buf = std::make_unique<int16_t[]>(buffer.size());
	memcpy(buf.get(), buffer.data(), buffer.size() * 2);
	return std::make_unique<SdlSoundData>(std::make_shared<SdlSoundDataWrapper>(std::move(buf), buffer.size()));
}

std::unique_ptr<Platform::SoundSource> SdlPlatform::play_sound(const SoundData &base, bool looping) {
	const SdlSoundData &snd = dynamic_cast<const SdlSoundData&>(base);

	std::shared_ptr<PlaybackState> state = std::make_shared<PlaybackState>();
	state->data = snd.get();
	state->position = 0;
	state->looping = looping;
	state->stop = false;

	if (state->data->samples == 0)
		return nullptr;

	SDL_LockAudioDevice(audio_device);
	playback_list.push_back(state);
	SDL_UnlockAudioDevice(audio_device);

	return std::make_unique<SdlSoundSource>(state);
}

int64_t SdlPlatform::get_time()
{
	return std::time(nullptr);
}
