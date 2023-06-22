set (SOURCES evc.cpp DMI/dmi.cpp Supervision/national_values.cpp Supervision/fixed_values.cpp 
Supervision/curve_calc.cpp  Supervision/conversion_model.cpp  Position/distance.cpp
Supervision/speed_profile.cpp Supervision/supervision.cpp Supervision/targets.cpp Supervision/train_data.cpp 
Supervision/emergency_stop.cpp 
Supervision/acceleration.cpp antenna.cpp MA/movement_authority.cpp MA/mode_profile.cpp Position/linking.cpp 
OR_interface/interface.cpp OR_interface/orts_server.cpp SSP/ssp.cpp Packets/packets.cpp Procedures/mode_transition.cpp LX/level_crossing.cpp
Packets/messages.cpp Packets/information.cpp Packets/radio.cpp Packets/vbc.cpp Euroradio/session.cpp Euroradio/terminal.cpp 
Packets/logging.cpp Packets/io/io.cpp Packets/io/base64.cpp STM/stm.cpp Packets/STM/message.cpp
Procedures/start.cpp Procedures/override.cpp Procedures/train_trip.cpp Procedures/level_transition.cpp 
Procedures/stored_information.cpp TrackConditions/track_conditions.cpp  TrackConditions/route_suitability.cpp
Time/clock.cpp Position/geographical.cpp DMI/text_messages.cpp DMI/windows.cpp DMI/track_ahead_free.cpp
TrainSubsystems/power.cpp TrainSubsystems/brake.cpp TrainSubsystems/train_interface.cpp TrainSubsystems/cold_movement.cpp
language/language.cpp Version/version.cpp Version/translate.cpp Config/config.cpp
NationalFN/nationalfn.cpp NationalFN/asfa.cpp
../platform/platform.cpp
../libs/liborts/serverlib.cpp ../libs/liborts/common.cpp
)

if (NOT WASM)
    list(APPEND SOURCES ../platform/console_platform.cpp ../platform/console_fd_poller.cpp ../platform/tcp_socket.cpp ../platform/bus_socket_impl.cpp ../platform/tcp_listener.cpp ../platform/libc_time_impl.cpp ../platform/fstream_file_impl.cpp ../platform/bus_socket_server.cpp ../platform/bus_tcp_bridge.cpp)
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

add_definitions(-DEVC)

if(WIN32)
    target_link_libraries(evc PRIVATE imagehlp ws2_32 psapi)
endif()
if(ANDROID)
    target_link_libraries(evc PRIVATE log)
endif()
