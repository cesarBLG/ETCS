 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "renderer.h"

class NullRenderer : public Renderer {
public:
	class NullImage : public Image
	{
	public:
		NullImage() = default;
		virtual int width() const override { return 0; };
		virtual int height() const override { return 0; };
	};

	class NullFont : public Font
	{
	public:
		NullFont() = default;
		virtual float ascent() const override { return 0.0f; }
		virtual std::pair<float, float> calc_size(const std::string &str) const override { return std::make_pair(0.0f, 0.0f); }
	};

	NullRenderer() = default;
	virtual void set_color(Color c) override {};
	virtual void draw_line(int x1, int y1, int x2, int y2) override {};
	virtual void draw_rect(int x, int y, int w, int h) override {};
	virtual void draw_rect_filled(int x, int y, int w, int h) override {};
	virtual void draw_image(const Image &img, int x, int y, int w, int h) override {};
	virtual void draw_circle_filled(int x, int y, int rad) override {};
	virtual void draw_polygon_filled(const short int *vx, const short int *vy, size_t n) override {};
	virtual void clear() override {};
	virtual std::unique_ptr<Image> load_image(const std::string &path) override { return std::make_unique<NullImage>(); };
	virtual std::unique_ptr<Font> load_font(float size, bool bold) override { return std::make_unique<NullFont>(); };
	virtual std::unique_ptr<Image> make_text_image(const std::string &text, const Font &font, Color c) override { return std::make_unique<NullImage>(); };
	virtual std::unique_ptr<Image> make_wrapped_text_image(const std::string &text, const Font &font, int align, Color c) override { return std::make_unique<NullImage>(); };
};
