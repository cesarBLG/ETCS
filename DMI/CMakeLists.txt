set (SOURCES init.cpp monitor.cpp time.cpp graphics/sdl/drawing.cpp graphics/sdl/component.cpp graphics/button.cpp 
            graphics/display.cpp window/window.cpp graphics/layout.cpp graphics/text_button.cpp graphics/icon_button.cpp 
            speed/gauge.cpp sound/sound.cpp distance/distance.cpp tcp/server.cpp state/level.cpp state/mode.cpp 
            state/brake.cpp state/conditions.cpp state/acks.cpp state/override.cpp state/radio.cpp 
            messages/messages.cpp graphics/flash.cpp 
            window/window_main.cpp state/time_hour.cpp window/subwindow.cpp window/data_entry.cpp window/data_validation.cpp 
            window/nav_buttons.cpp window/menu.cpp window/menu_main.cpp window/menu_override.cpp window/menu_radio.cpp 
            window/menu_spec.cpp window/menu_settings.cpp window/set_vbc.cpp window/track_ahead_free.cpp 
            window/driver_id.cpp window/running_number.cpp window/level_window.cpp window/sr_data.cpp 
            window/rbc_data.cpp window/input_data.cpp window/fixed_train_data.cpp window/keyboard.cpp 
            window/train_data.cpp 
            planning/planning.cpp control/control.cpp state/gps_pos.cpp 
            graphics/sdl/gfx_primitives.cpp
)


if (NOT ETCS_VENDORED)
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)       
    find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
    find_package(SDL2_ttf REQUIRED CONFIG REQUIRED COMPONENTS SDL2_ttf)
endif()

add_executable(dmi ${SOURCES})
target_compile_definitions(dmi PUBLIC _GLIBCXX_DEBUG)

if(TARGET SDL2::SDL2main)
    target_link_libraries(dmi PRIVATE SDL2::SDL2main)
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(dmi PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf)

if(WIN32)
    target_link_libraries(dmi PRIVATE imagehlp wsock32)
endif()