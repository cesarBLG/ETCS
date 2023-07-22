set (SOURCES init.cpp monitor.cpp time.cpp graphics/drawing.cpp graphics/component.cpp graphics/button.cpp 
            graphics/display.cpp window/window.cpp graphics/layout.cpp graphics/text_button.cpp graphics/icon_button.cpp 
            speed/gauge.cpp sound/sound.cpp distance/distance.cpp tcp/server.cpp state/level.cpp state/mode.cpp 
            state/brake.cpp state/conditions.cpp state/acks.cpp state/override.cpp state/radio.cpp 
            messages/messages.cpp graphics/flash.cpp 
            window/window_main.cpp state/time_hour.cpp window/subwindow.cpp window/data_entry.cpp window/data_validation.cpp 
            window/nav_buttons.cpp window/menu.cpp window/menu_main.cpp window/menu_override.cpp window/menu_radio.cpp 
            window/menu_spec.cpp window/menu_settings.cpp window/menu_ntc.cpp window/track_ahead_free.cpp 
            window/driver_id.cpp window/volume.cpp
            window/input_data.cpp window/fixed_train_data.cpp window/keyboard.cpp 
            window/train_data.cpp  STM/stm_objects.cpp ../EVC/Packets/STM/message.cpp
            planning/planning.cpp control/control.cpp state/gps_pos.cpp 
            language/language.cpp
            ../EVC/Packets/io/io.cpp ../EVC/Packets/io/base64.cpp
            Config/config.cpp
            ../platform/platform.cpp ../platform/platform_runtime.cpp
)

if(WIN32)
    list(APPEND SOURCES resource.rc)
endif()

add_definitions(-DNO_THREADS)

if (NOT WASM)
    set(WITH_SDL TRUE)
    list(APPEND SOURCES ../platform/tcp_socket.cpp ../platform/console_fd_poller.cpp ../platform/bus_socket_impl.cpp ../platform/libc_time_impl.cpp ../platform/fstream_file_impl.cpp)
else()
    set(WITH_SDL FALSE)
    list(APPEND SOURCES ../platform/simrail_platform.cpp)
    add_definitions(-DJSON_TEST_KEEP_MACROS=1 -DJSON_HAS_FILESYSTEM=0 -DJSON_HAS_EXPERIMENTAL_FILESYSTEM=0)
    add_definitions(-DNO_EXCEPTIONS)
    add_compile_options(-fno-exceptions)
    add_link_options(-mexec-model=reactor)
    add_link_options(-Wl,--export=init)
    add_link_options(-Wl,--export=alloc_mem)
    add_link_options(-Wl,--export=free_mem)
    add_link_options(-Wl,--export-table)
endif()

if (WITH_SDL)
    list(APPEND SOURCES ../platform/sdl_gfx/gfx_primitives.cpp  ../platform/sdl_platform.cpp)
    if (NOT ETCS_VENDORED)
        find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)       
        find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
        find_package(SDL2_ttf REQUIRED CONFIG REQUIRED COMPONENTS SDL2_ttf)
    endif()
endif()

if (ANDROID)
    add_library(dmi SHARED ${SOURCES})
elseif(WIN32)
	add_executable(dmi WIN32 ${SOURCES})
else()
    add_executable(dmi ${SOURCES})
endif()
target_compile_definitions(dmi PRIVATE NOMINMAX)

if (WITH_SDL)
    if(TARGET SDL2::SDL2main)
        target_link_libraries(dmi PRIVATE SDL2::SDL2main)
    endif()

    # Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
    target_link_libraries(dmi PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf)
endif()

target_include_directories(dmi PRIVATE ../include)
target_include_directories(dmi PRIVATE ../platform)

if(WIN32)
    target_link_libraries(dmi PRIVATE imagehlp ws2_32 psapi)
elseif(ANDROID)
    target_link_libraries(dmi PRIVATE log android GLESv1_CM GLESv2 OpenSLES)
endif()

if (WITH_SDL AND WIN32)
    add_custom_command(TARGET dmi POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2>              
        $<TARGET_FILE_DIR:dmi>)
    add_custom_command(TARGET dmi POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2_ttf>              
        $<TARGET_FILE_DIR:dmi>)
endif()
