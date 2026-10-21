 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#define _USE_MATH_DEFINES
#include <SDL3/SDL_main.h>
#include "sdl_platform.h"
#include "sdl_gfx/gfx_primitives.h"
#include "console_tools.h"
#include "platform_runtime.h"
#include <algorithm>
#include <fstream>
#include <cmath>

int main(int argc, char *argv[])
{
	std::vector<std::string> args;
	for (int i = 1; i < argc; i++)
		args.push_back(std::string(argv[i]));
#ifdef __ANDROID__
	android_external_storage_dir = std::string(SDL_AndroidGetExternalStoragePath())+"/";
#endif
	platform = std::make_unique<SdlPlatform>(platform_size_w, platform_size_h, args);
	on_platform_ready();
	static_cast<SdlPlatform*>(platform.get())->event_loop();
	return 0;
}

void SdlPlatform::SdlPlatform::load_config(const std::vector<std::string>& args)
{
	std::ifstream file(config_dir + "settings.ini", std::ios::binary);
	std::string line;

	while (std::getline(file, line)) {
		while (!line.empty() && line.back() == '\r' || line.back() == '\n')
			line.pop_back();
		int pos = line.find('=');
		if (pos == -1)
			continue;
		ini_items[line.substr(0, pos)] = line.substr(pos + 1);
	}

	for (std::string line : args) {
		while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
			line.pop_back();
		int pos = line.find('=');
		if (pos == -1)
			continue;
		ini_items[line.substr(0, pos)] = line.substr(pos + 1);
	}
}

std::string SdlPlatform::SdlPlatform::get_config(const std::string_view key)
{
	return get_config(key, "");
}

std::string SdlPlatform::SdlPlatform::get_config(const std::string_view key, const std::string_view def)
{
	auto it = ini_items.find(key);
	if (it == ini_items.end())
		return std::string(def);
	return it->second;
}
SdlPlatform::SdlPlatform(float virtual_w, float virtual_h, const std::vector<std::string> &args) :
	virtual_w(virtual_w),
	virtual_h(virtual_h),
	assets_dir(get_files_dir(ETCS_ASSET_FILE)),
	config_dir(get_files_dir(ETCS_CONFIG_FILE)),
	storage_dir(get_files_dir(ETCS_STORAGE_FILE)),
	bus_socket_impl(config_dir, poller, args),
	fstream_file_impl()
{
	setup_crash_handler();

	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	load_config(args);
	bool fullscreen = get_config("fullScreen") == "true";
	int display = std::stoi(get_config("display", "0"));
	int width = std::stoi(get_config("width", "800"));
	int height = std::stoi(get_config("height", "600"));
	int xpos = std::stoi(get_config("xpos", "0"));
	int ypos = std::stoi(get_config("ypos", "0"));
	bool borderless = get_config("borderless") == "true";
	rotate = get_config("rotateScreen") == "true";
	bool ontop = get_config("alwaysOnTop") == "true";
	bool hidecursor = get_config("hideCursor") == "true";
	touch = get_config("touch") == "true";
	std::string title = get_config("title") == "" ? "SdlPlatform" : get_config("title");

	int x = borderless ? xpos : (fullscreen ? SDL_WINDOWPOS_CENTERED_DISPLAY(display) : SDL_WINDOWPOS_UNDEFINED);
	int y = borderless ? ypos : (fullscreen ? SDL_WINDOWPOS_CENTERED_DISPLAY(display) : SDL_WINDOWPOS_UNDEFINED);

	SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, title.c_str());
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, x);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, y);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);
	if (borderless) SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true);
	if (fullscreen) SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true);
    sdlwindow = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

	#ifdef __unix__
		std::string icon_path = get_files_dir(ETCS_ASSET_FILE) + "DMI.bmp";
		SDL_Surface* icon = SDL_LoadBMP(icon_path.c_str());
		if (icon) {
			SDL_SetWindowIcon(sdlwindow, icon);
			SDL_DestroySurface(icon);
		}
	#endif

	if (ontop)
		SDL_SetWindowAlwaysOnTop(sdlwindow, true);
	if (hidecursor)
		SDL_HideCursor();

	sdlrend = SDL_CreateRenderer(sdlwindow, nullptr);
	SDL_SetRenderVSync(sdlrend, true);

	calc_scale();

	TTF_Init();

	audio_samplerate = 44100;
	const SDL_AudioSpec spec = { SDL_AUDIO_S16, 1, audio_samplerate };
    audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, mixer_func_proxy, this);
	audio_volume = 100;
	SDL_ResumeAudioStreamDevice(audio_stream);

	running = true;
	present_count = 0;

	PlatformUtil::DeferredFulfillment::list = &event_list;
}

SdlPlatform::~SdlPlatform() {
	timer_queue.clear();
	on_close_list.clear();
	on_quit_list.clear();
	on_present_list.clear();
	on_input_list.clear();
	while (PlatformUtil::DeferredFulfillment::execute());
	PlatformUtil::DeferredFulfillment::list = nullptr;

	loaded_fonts.clear();
	SDL_DestroyAudioStream(audio_stream);
	TTF_Quit();
	//SDL_DestroyRenderer(sdlrend);
	SDL_DestroyWindow(sdlwindow);
	SDL_Quit();
}

void SdlPlatform::calc_scale() {
	SDL_GetCurrentRenderOutputSize(sdlrend, &wx, &wy);
	int px,py;
	SDL_GetWindowSize(sdlwindow, &px, &py);
	dpiscale = wx / (float)px;
	float sx = wx / virtual_w;
	float sy = wy / virtual_h;
	s = std::min(sx, sy);
	if (sx > sy) {
		ox = (wx - wy * (virtual_w / virtual_h)) * 0.5f;
		oy = 0.0f;
	} else {
		ox = 0.0f;
		oy = (wy - wx * (virtual_h / virtual_w)) * 0.5f;
	}
	if (rotate) {
		s *= -1.0f;
		ox += wx - ox * 2.0f;
		oy += wy - oy * 2.0f;
	}
	SDL_SetDefaultTextureScaleMode(sdlrend, s == std::floor(s) ? SDL_SCALEMODE_NEAREST : SDL_SCALEMODE_LINEAR);
}

std::unique_ptr<SdlPlatform::BusSocket> SdlPlatform::open_socket(const std::string_view channel, uint32_t tid) {
	return bus_socket_impl.open_bus_socket(channel, tid);
}

int64_t SdlPlatform::get_timer() {
	return libc_time_impl.get_timer();
}

std::optional<std::string> SdlPlatform::read_file(const std::string_view path, FileType type) {
	return fstream_file_impl.read_file((type == ETCS_ASSET_FILE ? assets_dir : (type == ETCS_CONFIG_FILE ? config_dir : storage_dir)) + std::string(path));
}

bool SdlPlatform::write_file(const std::string_view path, const std::string_view contents) {
	return fstream_file_impl.write_file(storage_dir + std::string(path), contents);
}

void SdlPlatform::debug_print(const std::string_view msg) {
	SDL_Log("debug_print: %.*s", msg.size(), msg.data());
}

PlatformUtil::Promise<void> SdlPlatform::delay(int ms) {
	auto pair = PlatformUtil::PromiseFactory::create<void>();
	timer_queue.insert(std::make_pair(get_timer() + ms, std::move(pair.second)));
	return std::move(pair.first);
}

PlatformUtil::Promise<void> SdlPlatform::on_quit_request() {
	return on_close_list.create_and_add();
}

PlatformUtil::Promise<void> SdlPlatform::on_quit() {
	return on_quit_list.create_and_add();
}

PlatformUtil::Promise<SdlPlatform::InputEvent> SdlPlatform::on_input_event() {
	return on_input_list.create_and_add();
}

bool SdlPlatform::poll_sdl() {
	SDL_Event ev;
	if (SDL_PollEvent(&ev)) {
		if (ev.type == SDL_EVENT_QUIT || ev.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
		{
			on_close_list.fulfill_all(false);
		}
		else if (touch && (ev.type == SDL_EVENT_FINGER_DOWN || ev.type == SDL_EVENT_FINGER_UP || ev.type == SDL_EVENT_FINGER_MOTION))
		{
			SDL_TouchFingerEvent tfe = ev.tfinger;
			InputEvent ev;
			if (tfe.type == SDL_EVENT_FINGER_MOTION)
			{
				if (tfe.pressure > 0)
				{
					ev.action = InputEvent::Action::Move;
					ev.x = (tfe.x*wx - ox) / s;
					ev.y = (tfe.y*wy - oy) / s;
					on_input_list.fulfill_all(ev, false);
				}
			}
			else
			{
				ev.action = tfe.pressure > 0 ? InputEvent::Action::Press : InputEvent::Action::Release;
				ev.x = (tfe.x*wx - ox) / s;
				ev.y = (tfe.y*wy - oy) / s;
				on_input_list.fulfill_all(ev, false);
			}
		}
		else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN || ev.type == SDL_EVENT_MOUSE_BUTTON_UP)
		{
			SDL_MouseButtonEvent sdlev = ev.button;
			if (sdlev.button == SDL_BUTTON_LEFT) {
				InputEvent ev;
				ev.action = sdlev.down ? InputEvent::Action::Press : InputEvent::Action::Release;
				ev.x = (sdlev.x * dpiscale - ox) / s;
				ev.y = (sdlev.y * dpiscale - oy) / s;

				on_input_list.fulfill_all(ev, false);
			}
		}
		else if (ev.type == SDL_EVENT_MOUSE_MOTION)
		{
			SDL_MouseMotionEvent sdlev = ev.motion;
			if (sdlev.state & SDL_BUTTON_LMASK) {
				InputEvent ev;
				ev.action = InputEvent::Action::Move;
				ev.x = (sdlev.x * dpiscale - ox) / s;
				ev.y = (sdlev.y * dpiscale - oy) / s;

				on_input_list.fulfill_all(ev, false);
			}
		}
        else if (ev.type == SDL_EVENT_WINDOW_RESIZED || ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
		{
            calc_scale();
			loaded_fonts.clear();
			void startWindows();
			startWindows();
        }
		return true;
	}
	return false;
}

void SdlPlatform::event_loop() {
	while (running) {
		bool idle = true;

		while (poll_sdl())
			idle = false;

		int64_t now = get_timer();
		if (!timer_queue.empty() && timer_queue.begin()->first <= now) {
			idle = false;
			timer_queue.begin()->second.fulfill(false);
			timer_queue.erase(timer_queue.begin());
		}

		for (int i = 0; i < 10; i++)
			if (PlatformUtil::DeferredFulfillment::execute())
				idle = false;
			else
				break;

		on_present_list.fulfill_all(false);

		if (present_count > 0) {
			present_count--;
			idle = false;
			SDL_RenderPresent(sdlrend);
			SDL_SetRenderDrawColor(sdlrend, 0, 0, 0, 255);
			SDL_RenderClear(sdlrend);
		}

		int64_t diff = -1;
		if (!timer_queue.empty())
			diff = std::max((int64_t)0, timer_queue.begin()->first - get_timer());

		if (diff == -1 || diff > 10)
			diff = 10;

		poller.poll(idle ? diff : 0);
	};

	on_quit_list.fulfill_all(false);
}

void SdlPlatform::quit() {
	running = false;
}

void SdlPlatform::set_color(Color c) {
	SDL_SetRenderDrawColor(sdlrend, c.R, c.G, c.B, 255);
	current_color = c;
}

void SdlPlatform::draw_line(float x1, float y1, float x2, float y2) {
	SDL_RenderLine(sdlrend, x1 * s + ox, y1 * s + oy, x2 * s + ox, y2 * s + oy);
}

void SdlPlatform::draw_rect(float x, float y, float w, float h) {
	SDL_FRect rect { x * s + ox, y * s + oy, w * s, h * s };
	SDL_RenderRect(sdlrend, &rect);
}

void SdlPlatform::draw_rect_filled(float x, float y, float w, float h) {
	SDL_FRect rect { x * s + ox, y * s + oy, w * s, h * s };
	SDL_RenderFillRect(sdlrend, &rect);
}

void SdlPlatform::draw_image(const Image &base, float x, float y) {
	const SdlImage &img = dynamic_cast<const SdlImage&>(base);
	auto size = img.size();
	if (s > 0.0f) {
		SDL_FRect rect { std::floor(x * s + ox), std::floor(y * s + oy), size.first * s, size.second * s };
		SDL_RenderTexture(sdlrend, img.get(), nullptr, &rect);
	} else {
		SDL_FRect rect { std::floor((x + size.first) * s + ox), std::floor((y + size.second) * s + oy), size.first * -s, size.second * -s };
		SDL_RenderTextureRotated(sdlrend, img.get(), nullptr, &rect, 180.0, nullptr, SDL_FLIP_NONE);
	}
}

void SdlPlatform::draw_circle_filled(float x, float y, float r) {
	Color c = current_color;
	filledCircleRGBA(sdlrend, x * s + ox, y * s + oy, r * s, c.R, c.G, c.B, 255);
	aacircleRGBA(sdlrend, x * s + ox, y * s + oy, r * s, c.R, c.G, c.B, 255);
}

void SdlPlatform::draw_arc_filled(float cx, float cy, float rmin, float rmax, float ang0, float ang1) {
    const int n = 51;
    std::vector<std::pair<float, float>> poly;
    poly.resize(2 * n);
    for(int i = 0; i < n; i++)
    {
        float an = ang0 + (ang1 - ang0) * i / (n - 1);
        float c = std::cos(an);
        float s = std::sin(an);
        poly[i].first = rmin * c + cx;
        poly[i].second = rmin * s + cy;
        poly[2 * n - 1 - i].first = rmax * c + cx;
        poly[2 * n - 1 - i].second = rmax * s + cy;
    }
    draw_polygon_filled(poly);
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

void SdlPlatform::draw_convex_polygon_filled(const std::vector<std::pair<float, float>> &poly) {
	draw_polygon_filled(poly);
}

PlatformUtil::Promise<void> SdlPlatform::on_present_request() {
	return on_present_list.create_and_add();
}

void SdlPlatform::present() {
	present_count++;
}

std::unique_ptr<SdlPlatform::Image> SdlPlatform::load_image(const std::string_view p) {
	std::string path = assets_dir + std::string(p);
	SDL_Surface *surf = SDL_LoadBMP(path.c_str());
	if (surf == nullptr) {
		printf("Error loading BMP %s. SDL Error: %s\n", path.c_str(), SDL_GetError());
		return nullptr;
	}
	SDL_Texture *tex = SDL_CreateTextureFromSurface(sdlrend, surf);
	if (tex == nullptr) {
		printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		SDL_DestroySurface(surf);
		return nullptr;
	}
	std::unique_ptr<SdlImage> img = std::make_unique<SdlImage>(tex, surf->w, surf->h, 1.0f);
	SDL_DestroySurface(surf);
	return img;
}

std::unique_ptr<SdlPlatform::Font> SdlPlatform::load_font(float ascent, bool bold, const std::string_view lang) {
#ifdef SIMRAIL
	ascent *= 1.25f;
#endif
	float scale = std::abs(s);

	std::string lang_str(lang);
	auto it = loaded_fonts.find({ascent, bold, lang_str});
	std::shared_ptr<SdlFontWrapper> wrapper;
	if (it != loaded_fonts.end())
		wrapper = it->second;

	if (!wrapper) {
		std::string path;
		TTF_Font* font;

#if SIMRAIL
		if (lang == "zh_Hans") {
			// Simplified Chinese
			path = assets_dir + (!bold ? "fonts/NotoSansSC-Regular.ttf" : "fonts/NotoSansSC-Bold.ttf");
		}
		else if (lang == "zh_Hant") {
			// Traditional  Chinese
			path = assets_dir + (!bold ? "fonts/NotoSansTC-Regular.ttf" : "fonts/NotoSansTC-Bold.ttf");
		}
		else {
			// Other languages
			path = assets_dir + (!bold ? "fonts/Play-Regular.ttf" : "fonts/Play-Bold.ttf");
		}
#else
		path = assets_dir + "fonts/" + (!bold ? get_config("font", "swiss.ttf") : get_config("fontBold", "swissb.ttf"));
#endif

		float size_probe = ascent * 2.0f * scale;
		font = TTF_OpenFont(path.c_str(), size_probe);
		if (font == nullptr) {
			debug_print("load_font failed: " + path);
			return nullptr;
		}

#if SIMRAIL
		float adjust = (ascent * scale) / TTF_GetFontAscent(font);
#else
		int minx, maxx, miny, maxy, advance;
		TTF_GetGlyphMetrics(font, 'R', &minx, &maxx, &miny, &maxy, &advance);

		float adjust = (ascent * scale) / maxy;
#endif
		TTF_CloseFont(font);

		font = TTF_OpenFont(path.c_str(), size_probe * adjust);
		if (font == nullptr) {
			debug_print("load_font failed: " + path);
			return nullptr;
		}

		wrapper = std::make_shared<SdlFontWrapper>(font);
		loaded_fonts.insert_or_assign({ascent, bold, lang_str}, wrapper);
	}

	return std::make_unique<SdlFont>(wrapper, scale);
}

std::unique_ptr<SdlPlatform::Image> SdlPlatform::make_text_image(const std::string_view text, const Font &base, Color c) {
	if (text.empty())
		return nullptr;

	const SdlFont &font = dynamic_cast<const SdlFont&>(base);

	SDL_Color color = { c.R, c.G, c.B };
	std::string text_str(text);
	SDL_Surface *surf = TTF_RenderText_Blended(font.get(), text_str.c_str(), text_str.length(), color);
	if (surf == nullptr) {
		printf("TTF render failed\n");
		return nullptr;
	}

	SDL_Texture *tex = SDL_CreateTextureFromSurface(sdlrend, surf);
	if (tex == nullptr) {
		printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		SDL_DestroySurface(surf);
		return nullptr;
	}

	std::unique_ptr<SdlImage> img = std::make_unique<SdlImage>(tex, surf->w, surf->h, std::abs(s));
	SDL_DestroySurface(surf);
	return img;
}

std::unique_ptr<SdlPlatform::Image> SdlPlatform::make_wrapped_text_image(const std::string_view text, const Font &base, float width, int align, Color c) {
	if (text.empty())
		return nullptr;

	const SdlFont &font = dynamic_cast<const SdlFont&>(base);

	TTF_SetFontWrapAlignment(font.get(), align == 0 ? TTF_HORIZONTAL_ALIGN_CENTER : (align == 1 ? TTF_HORIZONTAL_ALIGN_RIGHT : TTF_HORIZONTAL_ALIGN_LEFT));

	//if (aspect & 2) TTF_SetFontStyle(font, TTF_STYLE_UNDERLINE);
	SDL_Color color = { c.R, c.G, c.B };
	std::string text_str(text);
	SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(font.get(), text_str.c_str(), text_str.size(), color, width * std::abs(s));
	if (surf == nullptr) {
		printf("TTF render failed\n");
		return nullptr;
	}

	SDL_Texture *tex = SDL_CreateTextureFromSurface(sdlrend, surf);
	if (tex == nullptr) {
		printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		SDL_DestroySurface(surf);
		return nullptr;
	}

	std::unique_ptr<SdlImage> img = std::make_unique<SdlImage>(tex, surf->w, surf->h, std::abs(s));
	SDL_DestroySurface(surf);
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

std::pair<float, float> SdlPlatform::SdlImage::size() const {
	return std::make_pair(w / scale, h / scale);
}

SdlPlatform::SdlFontWrapper::SdlFontWrapper(TTF_Font *f) : font(f) {

}

SdlPlatform::SdlFontWrapper::~SdlFontWrapper() {
	TTF_CloseFont(font);
}

SdlPlatform::SdlFont::SdlFont(std::shared_ptr<SdlFontWrapper> wrapper, float s) : font(wrapper), scale(s) {

}

std::pair<float, float> SdlPlatform::SdlFont::calc_size(const std::string_view str, float wrap_width) const {
	if (wrap_width == 0.0f) {
		int w, h;
		std::string text_str(str);
		TTF_GetStringSize(font->font, text_str.c_str(), text_str.length(), &w, &h);
		return std::make_pair(w / scale, h / scale);
	} else {
		std::pair<float, float> out(0.0f, 0.0f);
		size_t pos = 0;
		while (pos != str.size()) {
			size_t p = calc_wrap_point(str.substr(pos), wrap_width);
			std::pair<float, float> line = calc_size(str.substr(pos, p));
			out.first = std::max(out.first, line.first);
			out.second += line.second;
			pos += p;
		}
		return out;
	}
}

size_t SdlPlatform::SdlFont::calc_wrap_point(const std::string_view str, float wrap_width) const {
	int extent;
	size_t count;
	std::string text_str(str);
	TTF_MeasureString(font->font, text_str.c_str(), text_str.length(), wrap_width * scale, &extent, &count);
	count = std::max(count, (size_t)1);
	if (count > str.size())
		return str.size();
	const char *pos = str.data();
	for (int i = 0; i < count; i++)  {
		char c = *pos;
		if ((c & 0b11111000) == 0b11110000)
			pos += 4;
		else if ((c & 0b11110000) == 0b11100000)
			pos += 3;
		else if ((c & 0b11100000) == 0b11000000)
			pos += 2;
		else
			pos += 1;
	}
	while (pos != str.data() + str.size() && *pos == ' ')
		pos++;
	return pos - str.data();
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
void SDLCALL SdlPlatform::mixer_func_proxy(void *ptr, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
	/* Calculate a little more audio here, maybe using `userdata`, write it to `stream`
		*
		* If you want to use the original callback, you could do something like this:
		*/
	if (additional_amount > 0) {
		Uint8 *data = SDL_stack_alloc(Uint8, additional_amount);
		if (data) {
			((SdlPlatform*)ptr)->mixer_func((int16_t*)data, additional_amount / 2);
			SDL_PutAudioStreamData(stream, data, additional_amount);
			SDL_stack_free(data);
		}
	}
}

void SdlPlatform::mixer_func(int16_t *buffer, size_t len) {
	memset(buffer, 0, len * 2);

	playback_list.erase(std::remove_if(playback_list.begin(), playback_list.end(), [=](const std::shared_ptr<PlaybackState> &state)
	{
		if (state->stop)
			return true;

		size_t i = 0;
		while (i < len) {
			buffer[i++] = std::clamp(buffer[i] + state->data->buffer[state->position++] * audio_volume / 100, (int)INT16_MIN, (int)INT16_MAX);

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

void SdlPlatform::set_volume(int vol) {
	audio_volume = vol;
}

int SdlPlatform::get_volume() {
	return audio_volume;
}

std::unique_ptr<SdlPlatform::SoundData> SdlPlatform::load_sound(const std::string_view path) {
	std::string file =
        assets_dir + "sound/" + std::string(path) + ".wav";

    SDL_AudioSpec spec;
    Uint8 *buffer = nullptr;
    Uint32 len = 0;

    if (!SDL_LoadWAV(file.c_str(), &spec, &buffer, &len)) {
        return nullptr;
    }

    SDL_AudioSpec dst;
    dst.format   = SDL_AUDIO_S16;
    dst.channels = 1;
    dst.freq     = audio_samplerate;

    SDL_AudioStream *stream = SDL_CreateAudioStream(&spec, &dst);
    if (!stream) {
        SDL_free(buffer);
		debug_print("SDL_CreateAudioStream");
        return nullptr;
    }

    if (!SDL_PutAudioStreamData(stream, buffer, (int)len)) {
        SDL_free(buffer);
        SDL_DestroyAudioStream(stream);
		debug_print("SDL_PutAudioStreamData");
        return nullptr;
    }

    SDL_free(buffer);
    SDL_FlushAudioStream(stream);

    size_t available = SDL_GetAudioStreamAvailable(stream);
    if (available == 0) {
        SDL_DestroyAudioStream(stream);
		debug_print("SDL_GetAudioStreamAvailable");
        return nullptr;
    }

    size_t samples = available / sizeof(int16_t);

    auto buf = std::make_unique<int16_t[]>(samples);

    size_t got = SDL_GetAudioStreamData(stream, buf.get(), (int)available);
    SDL_DestroyAudioStream(stream);

    if (got == 0) {
        return nullptr;
    }

	return std::make_unique<SdlSoundData>(std::make_shared<SdlSoundDataWrapper>(std::move(buf), samples));
}

std::unique_ptr<SdlPlatform::SoundData> SdlPlatform::load_sound(const std::vector<std::pair<int, int>> &melody) {
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

std::unique_ptr<SdlPlatform::SoundSource> SdlPlatform::play_sound(const SoundData &base, bool looping) {
	const SdlSoundData &snd = dynamic_cast<const SdlSoundData&>(base);

	std::shared_ptr<PlaybackState> state = std::make_shared<PlaybackState>();
	state->data = snd.get();
	state->position = 0;
	state->looping = looping;
	state->stop = false;

	if (state->data->samples == 0)
		return nullptr;

	playback_list.push_back(state);

	return std::make_unique<SdlSoundSource>(state);
}

void SdlPlatform::set_brightness(int val) {
	//SDL_SetWindowBrightness(sdlwindow, val / 100.0f);
}

int SdlPlatform::get_brightness() {
	return 100;//(int)(SDL_GetWindowBrightness(sdlwindow) * 100);
}
