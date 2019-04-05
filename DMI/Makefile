OBJECTS = init.o monitor.o time.o graphics/sdl/drawing.o graphics/sdl/component.o graphics/button.o \
		graphics/display.o graphics/window.o graphics/layout.o graphics/text_button.o graphics/icon_button.o \
		speed/gauge.o sound/sound.o distance/distance.o tcp/server.o state/level.o state/mode.o \
		state/brake.o state/conditions.o state/acks.o messages/messages.o graphics/flash.o \
		graphics/window_main.o graphics/time_hour.o graphics/subwindow.o  graphics/data_entry.o\
		graphics/nav_buttons.o graphics/menu.o graphics/menu_main.o graphics/menu_override.o \
		graphics/menu_spec.o graphics/menu_settings.o graphics/running_number.o \
		graphics/level_window.o \
		planning/planning.o
all: dmi
dmi: $(OBJECTS)
	g++ $(OBJECTS) -o dmi -lSDL2 -lSDL2_ttf -lSDL2_net -lSDL2_gfx -pthread
%.o: %.cpp
	g++ -c $< -o $@ 
clean:
	rm $(OBJECTS)
