set (SOURCES evc.cpp DMI/dmi.cpp Supervision/national_values.cpp Supervision/fixed_values.cpp 
Supervision/curve_calc.cpp  Supervision/conversion_model.cpp  Position/distance.cpp
Supervision/speed_profile.cpp Supervision/supervision.cpp Supervision/targets.cpp Supervision/train_data.cpp Supervision/locomotive_data.cpp 
Supervision/emergency_stop.cpp 
Supervision/acceleration.cpp MA/movement_authority.cpp MA/mode_profile.cpp Position/linking.cpp 
OR_interface/interface.cpp OR_interface/orts_server.cpp SSP/ssp.cpp Packets/packets.cpp Procedures/mode_transition.cpp LX/level_crossing.cpp
Packets/messages.cpp Packets/information.cpp Packets/radio.cpp Packets/vbc.cpp 
Euroradio/session.cpp Euroradio/terminal.cpp Euroradio/radio_connection.cpp Euroradio/safe_radio_connection.cpp Euroradio/cfm.cpp
Packets/logging.cpp Packets/io/io.cpp Packets/io/base64.cpp STM/stm.cpp Packets/STM/message.cpp
Procedures/start.cpp Procedures/override.cpp Procedures/train_trip.cpp Procedures/level_transition.cpp Procedures/reversing.cpp
Procedures/stored_information.cpp TrackConditions/track_conditions.cpp  TrackConditions/route_suitability.cpp
Time/clock.cpp Position/geographical.cpp DMI/text_messages.cpp DMI/windows.cpp DMI/track_ahead_free.cpp DMI/acks.cpp
TrainSubsystems/power.cpp TrainSubsystems/brake.cpp TrainSubsystems/train_interface.cpp TrainSubsystems/cold_movement.cpp TrainSubsystems/asc.cpp
language/language.cpp Version/version.cpp Version/translate.cpp Config/config.cpp
NationalFN/nationalfn.cpp NationalFN/asfa.cpp
../platform/platform.cpp ../platform/platform_runtime.cpp
../libs/liborts/serverlib.cpp ../libs/liborts/common.cpp
)

add_definitions(-DNO_THREADS)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (LINUX)
        add_definitions(-D_GLIBCXX_DEBUG)
    endif()
endif()

if (DEBUG_VERBOSE)
    add_definitions(-DDEBUG_MSG_CONSISTENCY -DDEBUG_ODOMETER -DDEBUG_MODE -DDEBUG_STM)
    if (NOT SIMRAIL)
        add_definitions(-DDEBUG_TRACK_MESSAGES)
    endif()
endif()

if (RADIO_CFM)
    list (APPEND SOURCES Euroradio/tcp_cfm.cpp)
endif()

if (NOT WASM)
    list(APPEND SOURCES ../platform/console_platform.cpp ../platform/console_tools.cpp ../platform/console_fd_poller.cpp ../platform/tcp_socket.cpp ../platform/bus_socket_impl.cpp ../platform/tcp_listener.cpp ../platform/libc_time_impl.cpp ../platform/fstream_file_impl.cpp ../platform/bus_socket_server.cpp ../platform/bus_tcp_bridge.cpp ../platform/orts_bridge.cpp ../libs/liborts/ip_discovery.cpp)
else()
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

if(WIN32)
    list(APPEND SOURCES resource.rc)
endif()

if (ANDROID)
    add_library(evc SHARED ${SOURCES})
else()
    add_executable(evc ${SOURCES})
endif()
target_compile_definitions(evc PRIVATE NOMINMAX)
target_include_directories(evc PRIVATE ../include)
target_include_directories(evc PRIVATE ../platform)
target_include_directories(evc PRIVATE ../libs/liborts/include)
target_include_directories(evc PRIVATE ../libs/imgui)

add_definitions(-DEVC)


if (RADIO_CFM)
    if (ETCS_VENDORED)
        set (ETCS_CARES_VENDORED ON)
    else()
        find_package(c-ares CONFIG)
        if (c-ares_FOUND)
            set(ETCS_CARES_VENDORED OFF)
        else()
            set(ETCS_CARES_VENDORED ON)
        endif()
    endif()
    if (ETCS_CARES_VENDORED)
        add_subdirectory(../libs/c-ares EXCLUDE_FROM_ALL cares)
    endif()
    target_link_libraries(evc PRIVATE c-ares::cares)
endif()

if(WIN32)
    target_link_libraries(evc PRIVATE imagehlp ws2_32 psapi)
endif()
if(ANDROID)
    target_link_libraries(evc PRIVATE log)
endif()
if (WASM)
    set_target_properties(evc PROPERTIES SUFFIX ".wasm")
endif()

if (NOT ANDROID AND NOT WASM)
    install(TARGETS evc RUNTIME)
    install(DIRECTORY TrainData DESTINATION ${ETCS_ASSET_DIR})
    if (RADIO_CFM AND ETCS_CARES_VENDORED AND ETCS_PACK_VENDORED)
        if (WIN32)
            install(TARGETS c-ares RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
        else()
            install(TARGETS c-ares LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endif()
    endif()
    install(DIRECTORY ../locales/evc DESTINATION ${ETCS_ASSET_DIR}/locales)
    install(FILES config.json DESTINATION ${ETCS_ASSET_DIR})
endif()