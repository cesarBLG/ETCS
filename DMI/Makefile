CPP = g++
SOURCES = init.cpp monitor.cpp time.cpp graphics/sdl/drawing.cpp graphics/sdl/component.cpp graphics/button.cpp \
		graphics/display.cpp window/window.cpp graphics/layout.cpp graphics/text_button.cpp graphics/icon_button.cpp \
		speed/gauge.cpp sound/sound.cpp distance/distance.cpp tcp/server.cpp state/level.cpp state/mode.cpp \
		state/brake.cpp state/conditions.cpp state/acks.cpp state/override.cpp state/radio.cpp \
		messages/messages.cpp graphics/flash.cpp \
		window/window_main.cpp state/time_hour.cpp window/subwindow.cpp window/data_entry.cpp window/data_validation.cpp \
		window/nav_buttons.cpp window/menu.cpp window/menu_main.cpp window/menu_override.cpp window/menu_radio.cpp \
		window/menu_spec.cpp window/menu_settings.cpp window/set_vbc.cpp window/track_ahead_free.cpp \
		window/driver_id.cpp window/running_number.cpp window/level_window.cpp window/sr_data.cpp \
		window/rbc_data.cpp window/input_data.cpp window/fixed_train_data.cpp window/keyboard.cpp \
		window/train_data.cpp \
		planning/planning.cpp control/control.cpp state/gps_pos.cpp \
		graphics/sdl/gfx_primitives.cpp
CFLAGS = -Wall -g
LDFLAGS = -lSDL2 -lSDL2_ttf -pthread
ifeq ($(TARGET),WIN32)
EXE = dmi.exe
CPP = x86_64-w64-mingw32-g++ -m64 -mwindows
LDFLAGS = -mwindows -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lwsock32 -limagehlp
BUILD_DIR = build/win32/
else
EXE = dmi
CFLAGS += -rdynamic
BUILD_DIR = build/linux/
endif
OBJECTS = $(patsubst %.cpp,%.o,$(addprefix $(BUILD_DIR), $(SOURCES)))
all: $(EXE)
$(EXE): $(OBJECTS)
	$(CPP) $(OBJECTS) -o dmi $(CFLAGS) $(LDFLAGS)
$(BUILD_DIR)%.o: %.cpp
	@mkdir -p $(@D)
	$(CPP) -c $< -o $@ $(CFLAGS)
clean:
	rm $(OBJECTS)
