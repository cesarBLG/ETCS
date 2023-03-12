set (SOURCES evc.cpp DMI/dmi.cpp Supervision/national_values.cpp Supervision/fixed_values.cpp 
Supervision/curve_calc.cpp  Supervision/conversion_model.cpp  Position/distance.cpp
Supervision/speed_profile.cpp Supervision/supervision.cpp Supervision/targets.cpp Supervision/train_data.cpp 
Supervision/emergency_stop.cpp 
Supervision/acceleration.cpp antenna.cpp MA/movement_authority.cpp MA/mode_profile.cpp Position/linking.cpp 
OR_interface/interface.cpp SSP/ssp.cpp Packets/packets.cpp Procedures/mode_transition.cpp LX/level_crossing.cpp 
Packets/messages.cpp Packets/radio.cpp Packets/vbc.cpp Euroradio/session.cpp Euroradio/terminal.cpp 
Packets/logging.cpp Packets/io/io.cpp Packets/io/base64.cpp STM/stm.cpp Packets/STM/message.cpp
Procedures/start.cpp Procedures/override.cpp Procedures/train_trip.cpp Procedures/level_transition.cpp 
Procedures/stored_information.cpp TrackConditions/track_conditions.cpp Time/clock.cpp Position/geographical.cpp 
DMI/text_messages.cpp DMI/windows.cpp DMI/track_ahead_free.cpp TrainSubsystems/power.cpp TrainSubsystems/brake.cpp
language/language.cpp
NationalFN/nationalfn.cpp NationalFN/asfa.cpp
)

if(WIN32)
    list(APPEND SOURCES resource.rc)
endif()

if (ANDROID)
    add_library(evc SHARED ${SOURCES})
else()
    add_executable(evc ${SOURCES})
endif()
target_compile_definitions(evc PUBLIC _GLIBCXX_DEBUG)
target_include_directories(evc PRIVATE ../include)
target_link_libraries(evc PRIVATE orts)

if(WIN32)
    target_link_libraries(evc PRIVATE imagehlp wsock32)
endif()
if(ANDROID)
    target_link_libraries(evc PRIVATE log)
endif()
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(evc PRIVATE Threads::Threads)

if(WIN32)
    add_custom_command(TARGET evc POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:orts>              
        $<TARGET_FILE_DIR:evc>)
    add_custom_command(TARGET evc POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:server>              
        $<TARGET_FILE_DIR:evc>)
endif()