 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include "../graphics/color.h"

class Platform
{
public:
	class Image
	{
	public:
		virtual ~Image() = default;
		virtual float width() const = 0;
		virtual float height() const = 0;
	};

	class Font
	{
	public:
		virtual ~Font() = default;
		virtual float ascent() const = 0;
		virtual std::pair<float, float> calc_size(const std::string &str) const = 0;
	};

	class SoundData
	{
	public:
		virtual ~SoundData() = default;
	};

	class SoundSource
	{
	public:
		virtual ~SoundSource() = default;
		virtual void detach() = 0;
	};

	virtual ~Platform() = default;

	virtual void set_color(Color c) = 0;
	virtual void draw_line(float x1, float y1, float x2, float y2) = 0;
	virtual void draw_rect(float x, float y, float w, float h) = 0;
	virtual void draw_rect_filled(float x, float y, float w, float h) = 0;
	virtual void draw_image(const Image &img, float x, float y, float w, float h) = 0;
	virtual void draw_circle_filled(float x, float y, float rad) = 0;
	virtual void draw_polygon_filled(const std::vector<std::pair<float, float>> &poly) = 0;
	virtual void clear() = 0;
	virtual std::unique_ptr<Image> load_image(const std::string &path) = 0;
	virtual std::unique_ptr<Font> load_font(float size, bool bold) = 0;
	virtual std::unique_ptr<Image> make_text_image(const std::string &text, const Font &font, Color c) = 0;
	virtual std::unique_ptr<Image> make_wrapped_text_image(const std::string &text, const Font &font, int align, Color c) = 0;

	virtual std::unique_ptr<SoundData> load_sound(const std::string &path) = 0;
	virtual std::unique_ptr<SoundData> load_sound(const std::vector<std::pair<int, int>> &melody) = 0;
	virtual std::unique_ptr<SoundSource> play_sound(const SoundData &snd, bool looping) = 0;

	virtual int64_t get_time() = 0;
};

extern std::unique_ptr<Platform> platform;
