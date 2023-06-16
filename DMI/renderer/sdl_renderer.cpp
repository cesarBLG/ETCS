 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "sdl_renderer.h"
#include "../graphics/drawing.h"
#include "../graphics/sdl/gfx_primitives.h"
#include <SDL.h>
#include <SDL_ttf.h>

std::unique_ptr<Renderer> rend_backend;

SdlRenderer::SdlRenderer(SDL_Renderer *r) : sdlrend(r) {
#ifdef __ANDROID__
	extern std::string filesDir;
	load_path = filesDir + "/";
#endif
}

SdlRenderer::~SdlRenderer() {

}

void SdlRenderer::set_color(Color c) {
	SDL_SetRenderDrawColor(sdlrend, c.R, c.G, c.B, 255);
	current_color = c;
}

void SdlRenderer::draw_line(int x1, int y1, int x2, int y2) {
	SDL_RenderDrawLine(sdlrend, x1, y1, x2, y2);
}

void SdlRenderer::draw_rect(int x, int y, int w, int h) {
	SDL_Rect rect = { x, y, w, h };
	SDL_RenderDrawRect(sdlrend, &rect);
}

void SdlRenderer::draw_rect_filled(int x, int y, int w, int h) {
	SDL_Rect rect = { x, y, w, h };
	SDL_RenderFillRect(sdlrend, &rect);
}

void SdlRenderer::draw_image(const Image &base, int x, int y, int w, int h) {
	const SdlImage &img = dynamic_cast<const SdlImage&>(base);
	SDL_Rect rect = SDL_Rect({x, y, w, h});
	SDL_RenderCopy(sdlrend, img.get(), nullptr, &rect);
}

void SdlRenderer::draw_circle_filled(int x, int y, int rad) {
	Color c = current_color;
	filledCircleRGBA(sdlrend, x, y, rad, c.R, c.G, c.B, 255);
	aacircleRGBA(sdlrend, x, y, rad, c.R, c.G, c.B, 255);
}

void SdlRenderer::draw_polygon_filled(const int16_t *vx, const int16_t *vy, size_t n) {
	Color c = current_color;
	filledPolygonRGBA(sdlrend, vx, vy, n, c.R, c.G, c.B, 255);
	aapolygonRGBA(sdlrend, vx, vy, n, c.R, c.G, c.B, 255);
}

void SdlRenderer::clear() {
	SDL_RenderClear(sdlrend);
}

std::unique_ptr<Renderer::Image> SdlRenderer::load_image(const std::string &p) {
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
	std::unique_ptr<SdlImage> img = std::make_unique<SdlImage>(tex, surf->w, surf->h);
	SDL_FreeSurface(surf);
	return img;
}

std::unique_ptr<Renderer::Font> SdlRenderer::load_font(float size, bool bold) {
	auto it = loaded_fonts.find({size, bold});
	std::shared_ptr<SdlFontWrapper> wrapper;
	if (it != loaded_fonts.end())
		wrapper = it->second;

	if (!wrapper) {
#if SIMRAIL
		std::string path = load_path + (!bold ? "fonts/verdana.ttf" : "fonts/verdanab.ttf");
		TTF_Font* font = TTF_OpenFont(path.c_str(), getScale(size) * 1.25);
#else
		std::string path = load_path + (!bold ? "fonts/swiss.ttf" : "fonts/swissb.ttf");
		TTF_Font* font = TTF_OpenFont(path.c_str(), getScale(size) * 1.4);
#endif
		if (font == nullptr)
			return nullptr;
		wrapper = std::make_shared<SdlFontWrapper>(font);
		loaded_fonts.insert_or_assign({size, bold}, wrapper);
	}

	return std::make_unique<SdlFont>(wrapper);
}

std::unique_ptr<Renderer::Image> SdlRenderer::make_text_image(const std::string &text, const Font &base, Color c) {
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

	std::unique_ptr<SdlImage> img = std::make_unique<SdlImage>(tex, surf->w, surf->h);
	SDL_FreeSurface(surf);
	return img;
}

std::unique_ptr<Renderer::Image> SdlRenderer::make_wrapped_text_image(const std::string &text, const Font &base, int align, Color c) {
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

	std::unique_ptr<SdlImage> img = std::make_unique<SdlImage>(tex, surf->w, surf->h);
	SDL_FreeSurface(surf);
	return img;
}

SdlRenderer::SdlImage::SdlImage(SDL_Texture *tex, int w, int h) : tex(tex), w(w), h(h) {

}

SdlRenderer::SdlImage::~SdlImage() {
	SDL_DestroyTexture(tex);
}

SDL_Texture* SdlRenderer::SdlImage::get() const {
	return tex;
}

int SdlRenderer::SdlImage::width() const {
	return w;
}

int SdlRenderer::SdlImage::height() const {
	return h;
}

SdlRenderer::SdlFontWrapper::SdlFontWrapper(TTF_Font *f) : font(f) {

}

SdlRenderer::SdlFontWrapper::~SdlFontWrapper() {
	TTF_CloseFont(font);
}

TTF_Font* SdlRenderer::SdlFontWrapper::get() const {
	return font;
}

SdlRenderer::SdlFont::SdlFont(std::shared_ptr<SdlFontWrapper> wrapper) : font(wrapper) {

}

float SdlRenderer::SdlFont::ascent() const {
	return TTF_FontAscent(font->get());
}

std::pair<float, float> SdlRenderer::SdlFont::calc_size(const std::string &str) const {
	int w, h;
    TTF_SizeUTF8(font->get(), str.c_str(), &w, &h);
    return std::make_pair(getAntiScale(w), getAntiScale(h));
}

TTF_Font* SdlRenderer::SdlFont::get() const {
	return font->get();
}

std::unique_ptr<Renderer::Sound> SdlRenderer::load_sound(const std::string &path) {
	return nullptr;
}

std::unique_ptr<Renderer::Sound> SdlRenderer::load_sound(const std::vector<std::pair<int, int>> &melody) {
	return nullptr;
}

int SdlRenderer::play_sound(const Sound &snd, bool looping) {
	return 0;
}

void SdlRenderer::stop_sound(int handle) {

}
