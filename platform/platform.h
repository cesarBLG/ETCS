 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include "platform_util.h"

class BasePlatform : private PlatformUtil::NoCopy
{
public:
	virtual ~BasePlatform() = default;

	class BusSocket : private PlatformUtil::NoCopy
	{
	public:
		struct PeerId {
			uint32_t tid;
			uint32_t uid;
			static constexpr uint32_t fourcc(std::string_view p) {
			    return ((p.size() >= 1 ? p[0] : '_') << 24) | ((p.size() >= 2 ? p[1] : '_') << 16) | ((p.size() >= 3 ? p[2] : '_') << 8) | (p.size() >= 4 ? p[3] : '_');
			}
		};
		virtual ~BusSocket() = default;
		virtual void broadcast(const std::string &data) = 0;
		virtual void broadcast(uint32_t tid, const std::string &data) = 0;
		virtual void send_to(uint32_t uid, const std::string &data) = 0;
		virtual PlatformUtil::Promise<std::pair<PeerId, std::string>> receive() = 0;
		virtual PlatformUtil::Promise<PeerId> on_peer_join() = 0;
		virtual PlatformUtil::Promise<PeerId> on_peer_leave() = 0;
	};

	struct DateTime
	{
		int year;
		int month;
		int day;

		int hour;
		int minute;
		int second;
	};

	virtual int64_t get_timer() = 0;
	virtual int64_t get_timestamp() = 0;
	virtual DateTime get_local_time() = 0;

	virtual std::unique_ptr<BusSocket> open_socket(const std::string &bus, uint32_t tid) = 0;
	virtual std::string read_file(const std::string &path) = 0;
	virtual void write_file(const std::string &path, const std::string &contents) = 0;
	virtual void debug_print(const std::string &msg) = 0;

	virtual PlatformUtil::Promise<void> delay(int ms) = 0;
	virtual PlatformUtil::Promise<void> on_quit_request() = 0;
	virtual PlatformUtil::Promise<void> on_quit() = 0;

	virtual void quit() = 0;
};

class UiPlatform : public BasePlatform
{
public:
	class Image : private PlatformUtil::NoCopy
	{
	public:
		virtual ~Image() = default;
		virtual float width() const = 0;
		virtual float height() const = 0;
	};

	class Font : private PlatformUtil::NoCopy
	{
	public:
		virtual ~Font() = default;
		virtual float ascent() const = 0;
		virtual std::pair<float, float> calc_size(const std::string &str) const = 0;
	};

	class SoundData : private PlatformUtil::NoCopy
	{
	public:
		virtual ~SoundData() = default;
	};

	class SoundSource : private PlatformUtil::NoCopy
	{
	public:
		virtual ~SoundSource() = default;
		virtual void detach() = 0;
	};

	struct InputEvent
	{
		enum class Action {
			Press,
			Release,
			Move
		};
		Action action;
		float x, y;
	};

	struct Color {
		unsigned char R, G, B;
	};

	virtual void set_color(Color c) = 0;
	virtual void draw_line(float x1, float y1, float x2, float y2) = 0;
	virtual void draw_rect(float x, float y, float w, float h) = 0;
	virtual void draw_rect_filled(float x, float y, float w, float h) = 0;
	virtual void draw_image(const Image &img, float x, float y, float w, float h) = 0;
	virtual void draw_circle_filled(float x, float y, float rad) = 0;
	virtual void draw_polygon_filled(const std::vector<std::pair<float, float>> &poly) = 0;
	virtual void clear() = 0;
	virtual PlatformUtil::Promise<void> present() = 0;
	virtual std::unique_ptr<Image> load_image(const std::string &path) = 0;
	virtual std::unique_ptr<Font> load_font(float size, bool bold) = 0;
	virtual std::unique_ptr<Image> make_text_image(const std::string &text, const Font &font, Color c) = 0;
	virtual std::unique_ptr<Image> make_wrapped_text_image(const std::string &text, const Font &font, int align, Color c) = 0;

	virtual void set_volume(int vol) = 0;
	virtual std::unique_ptr<SoundData> load_sound(const std::string &path) = 0;
	virtual std::unique_ptr<SoundData> load_sound(const std::vector<std::pair<int, int>> &melody) = 0;
	virtual std::unique_ptr<SoundSource> play_sound(const SoundData &snd, bool looping) = 0;

	virtual void set_brightness(int br) = 0;

	virtual PlatformUtil::Promise<InputEvent> on_input_event() = 0;
};

extern float platform_size_w;
extern float platform_size_h;
void on_platform_ready();

#ifndef EVC
extern std::unique_ptr<UiPlatform> platform;
#else
extern std::unique_ptr<BasePlatform> platform;
#endif
