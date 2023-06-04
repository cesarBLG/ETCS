 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <memory>

class Renderer
{
public:
	struct Color
	{
		unsigned char R, G, B;
	};

	class Image
	{
	public:
		virtual ~Image() = default;
		virtual int width() const = 0;
		virtual int height() const = 0;
	};

	virtual ~Renderer() = default;
	virtual void set_color(Color c) = 0;
	virtual void draw_line(int x1, int y1, int x2, int y2) = 0;
	virtual void draw_rect(int x, int y, int w, int h) = 0;
	virtual void draw_rect_filled(int x, int y, int w, int h) = 0;
	virtual void draw_image(const Image &img, int x, int y, int w, int h) = 0;
	virtual void draw_circle_filled(int x, int y, int rad) = 0;
	virtual void draw_polygon_filled(const short int *vx, const short int *vy, size_t n) = 0;
	virtual std::unique_ptr<Image> load_image(const std::string &path) = 0;
	virtual std::unique_ptr<Image> make_text_image(const std::string &text, float size, int aspect, Color c) = 0;
	virtual std::unique_ptr<Image> make_wrapped_text_image(const std::string &text, float size, int aspect, int align, Color c) = 0;
};

extern std::unique_ptr<Renderer> rend_backend;
