#CPP = x86_64-w64-mingw32-g++
CPP = g++
OBJECTS = init.o monitor.o time.o graphics/sdl/drawing.o graphics/sdl/component.o graphics/button.o \
		graphics/display.o window/window.o graphics/layout.o graphics/text_button.o graphics/icon_button.o \
		speed/gauge.o sound/sound.o distance/distance.o tcp/server.o state/level.o state/mode.o \
		state/brake.o state/conditions.o state/acks.o state/override.o messages/messages.o graphics/flash.o \
		window/window_main.o graphics/time_hour.o window/subwindow.o  window/data_entry.o\
		window/nav_buttons.o window/menu.o window/menu_main.o window/menu_override.o \
		window/menu_spec.o window/menu_settings.o \
		window/driver_id.o window/running_number.o window/level_window.o \
		planning/planning.o control/control.o state/gps_pos.o \
		window/train_data.o window/keyboard.o window/input_data.o \
		graphics/sdl/gfx_primitives.o
all: dmi
dmi: $(OBJECTS)
#	$(CPP) $(OBJECTS) -o dmi -mwindows -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_net
	$(CPP) $(OBJECTS) -o dmi -g -lSDL2 -lSDL2_ttf -lSDL2_net -pthread
%.o: %.cpp
#	$(CPP) -c $< -o $@ -mwindows
	$(CPP) -c $< -o $@ -g -rdynamic
clean:
	rm $(OBJECTS)
