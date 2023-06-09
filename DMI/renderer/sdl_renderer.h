 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include "renderer.h"

struct SDL_Renderer;
struct SDL_Texture;
struct _TTF_Font;
typedef struct _TTF_Font TTF_Font;

class SdlRenderer : public Renderer {
private:
	class SdlFontWrapper
	{
	private:
		TTF_Font* font;
	public:
		SdlFontWrapper(TTF_Font *f);
		~SdlFontWrapper();
		TTF_Font* get() const;
	};

	SDL_Renderer *sdlrend;
	std::string load_path;
	Color current_color;
	std::map<std::pair<float, bool>, std::shared_ptr<SdlFontWrapper>> loaded_fonts;

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

	SdlRenderer(SDL_Renderer *r);
	virtual ~SdlRenderer() override;
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
};
