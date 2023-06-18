 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"

class NullPlatform : public UiPlatform {
public:
	class NullImage : public Image
	{
	public:
		NullImage() = default;
		virtual float width() const override { return 0.0f; };
		virtual float height() const override { return 0.0f; };
	};

	class NullFont : public Font
	{
	public:
		NullFont() = default;
		virtual float ascent() const override { return 0.0f; }
		virtual std::pair<float, float> calc_size(const std::string &str) const override { return std::make_pair(0.0f, 0.0f); }
	};

	class NullSoundData : public SoundData
	{
	public:
		NullSoundData() = default;
	};

	class NullSoundSource : public SoundSource
	{
	public:
		NullSoundSource() = default;
		virtual void detach() override {};
	};

	NullPlatform() = default;

	virtual int64_t get_timer() override { return 0; }
	virtual int64_t get_timestamp() override { return 0; }
	virtual DateTime get_local_time() override { return {}; }

	virtual std::string read_file(const std::string &path) override { return ""; };
	virtual void debug_print(const std::string &msg) override {};

	virtual PlatformUtil::Promise<void> delay(int ms) override { return {}; }
	virtual PlatformUtil::Promise<void> on_close() override { return {}; }

	virtual void event_loop() override {};

	virtual void set_color(Color c) override {};
	virtual void draw_line(float x1, float y1, float x2, float y2) override {};
	virtual void draw_rect(float x, float y, float w, float h) override {};
	virtual void draw_rect_filled(float x, float y, float w, float h) override {};
	virtual void draw_image(const Image &img, float x, float y, float w, float h) override {};
	virtual void draw_circle_filled(float x, float y, float rad) override {};
	virtual void draw_polygon_filled(const std::vector<std::pair<float, float>> &poly) override {};
	virtual void clear() override {};
	virtual PlatformUtil::Promise<void> present() override { return {}; };
	virtual std::unique_ptr<Image> load_image(const std::string &path) override { return std::make_unique<NullImage>(); };
	virtual std::unique_ptr<Font> load_font(float size, bool bold) override { return std::make_unique<NullFont>(); };
	virtual std::unique_ptr<Image> make_text_image(const std::string &text, const Font &font, Color c) override { return std::make_unique<NullImage>(); };
	virtual std::unique_ptr<Image> make_wrapped_text_image(const std::string &text, const Font &font, int align, Color c) override { return std::make_unique<NullImage>(); };

	virtual void set_volume(int vol) override {};
	virtual std::unique_ptr<SoundData> load_sound(const std::string &path) override { return std::make_unique<NullSoundData>(); };
	virtual std::unique_ptr<SoundData> load_sound(const std::vector<std::pair<int, int>> &melody) override { return std::make_unique<NullSoundData>(); };
	virtual std::unique_ptr<SoundSource> play_sound(const SoundData &snd, bool looping) override { return std::make_unique<NullSoundSource>(); };

	virtual void set_brightness(int br) override {};
};
