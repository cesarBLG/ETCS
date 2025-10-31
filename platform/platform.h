 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <cstdint>
#include "platform_util.h"
using std::uint32_t;
enum FileType
{
	ETCS_ASSET_FILE,
	ETCS_CONFIG_FILE,
	ETCS_STORAGE_FILE,
};
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
			static constexpr uint32_t fourcc(const std::string_view p) {
			    return ((p.size() >= 1 ? p[0] : '_') << 24) | ((p.size() >= 2 ? p[1] : '_') << 16) | ((p.size() >= 3 ? p[2] : '_') << 8) | (p.size() >= 4 ? p[3] : '_');
			}
			bool operator==(const PeerId &other) const {
				return uid == other.uid;
			}
			bool operator<(const PeerId &other) const {
				return uid < other.uid;
			}
		};
		struct JoinNotification { PeerId peer; };
		struct LeaveNotification { PeerId peer; };
		struct Message { PeerId peer; std::string data; };
		typedef std::variant<JoinNotification, LeaveNotification, Message> ReceiveResult;
		virtual ~BusSocket() = default;
		virtual void broadcast(const std::string_view data) = 0;
		virtual void broadcast(uint32_t tid, const std::string_view data) = 0;
		virtual void send_to(uint32_t uid, const std::string_view data) = 0;
		virtual PlatformUtil::Promise<ReceiveResult> receive() = 0;
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
	virtual DateTime get_local_time()
	{
		return {0,0,0,0,0,0};
	}

	virtual std::unique_ptr<BusSocket> open_socket(const std::string_view bus, uint32_t tid) = 0;
	virtual std::optional<std::string> read_file(const std::string_view path, FileType file_type=ETCS_ASSET_FILE) = 0;
	virtual bool write_file(const std::string_view path, const std::string_view contents) = 0;
	virtual void debug_print(const std::string_view msg) = 0;

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
		virtual std::pair<float, float> size() const = 0;
	};

	class Font : private PlatformUtil::NoCopy
	{
	public:
		virtual ~Font() = default;
		virtual std::pair<float, float> calc_size(const std::string_view str, float wrap_width = 0.0f) const = 0;
		virtual size_t calc_wrap_point(const std::string_view str, float wrap_width) const = 0;
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
	virtual void draw_image(const Image &img, float x, float y) = 0;
	virtual void draw_arc_filled(float x, float y, float r_min, float r_max, float a_min, float a_max) = 0;
	virtual void draw_circle_filled(float x, float y, float r) = 0;
	virtual void draw_convex_polygon_filled(const std::vector<std::pair<float, float>> &poly) = 0;
	virtual PlatformUtil::Promise<void> on_present_request() = 0;
	virtual void present() = 0;
	virtual std::unique_ptr<Image> load_image(const std::string_view path) = 0;
	virtual std::unique_ptr<Font> load_font(float ascent, bool bold, const std::string_view lang) = 0;
	virtual std::unique_ptr<Image> make_text_image(const std::string_view text, const Font &font, Color c) = 0;
	virtual std::unique_ptr<Image> make_wrapped_text_image(const std::string_view text, const Font &font, float width, int align, Color c) = 0;

	virtual void set_volume(int vol) = 0;
	virtual int get_volume() = 0;
	virtual std::unique_ptr<SoundData> load_sound(const std::string_view path) = 0;
	virtual std::unique_ptr<SoundData> load_sound(const std::vector<std::pair<int, int>> &melody) = 0;
	virtual std::unique_ptr<SoundSource> play_sound(const SoundData &snd, bool looping) = 0;

	virtual void set_brightness(int br) = 0;
	virtual int get_brightness() = 0;

	virtual PlatformUtil::Promise<InputEvent> on_input_event() = 0;
};

template <>
struct std::hash<BasePlatform::BusSocket::PeerId>
{
	std::size_t operator()(const BasePlatform::BusSocket::PeerId &id) const {
		return id.uid;
	}
};
