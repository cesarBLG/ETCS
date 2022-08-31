LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := dmi

SDL_PATH := SDL

LOCAL_C_INCLUDES := $(JNI_PATH)/$(SDL_PATH)/include $(JNI_PATH)/include

LOCAL_C_FLAGS +=

# Add your application source files here...
LOCAL_SRC_FILES := init.cpp monitor.cpp time.cpp graphics/sdl/drawing.cpp graphics/sdl/component.cpp graphics/button.cpp \
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

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
