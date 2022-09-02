set (SOURCES evc.cpp DMI/dmi.cpp Supervision/national_values.cpp Supervision/fixed_values.cpp 
Supervision/curve_calc.cpp  Supervision/conversion_model.cpp  Position/distance.cpp
Supervision/speed_profile.cpp Supervision/supervision.cpp Supervision/targets.cpp Supervision/train_data.cpp 
Supervision/emergency_stop.cpp 
Supervision/acceleration.cpp antenna.cpp MA/movement_authority.cpp MA/mode_profile.cpp Position/linking.cpp 
OR_interface/interface.cpp SSP/ssp.cpp Packets/packets.cpp Procedures/mode_transition.cpp LX/level_crossing.cpp 
Packets/messages.cpp Packets/radio.cpp Packets/vbc.cpp Euroradio/session.cpp Euroradio/terminal.cpp 
Packets/logging.cpp 
Procedures/start.cpp Procedures/override.cpp Procedures/train_trip.cpp Procedures/level_transition.cpp 
Procedures/stored_information.cpp TrackConditions/track_conditions.cpp Time/clock.cpp Position/geographical.cpp 
DMI/text_messages.cpp DMI/windows.cpp DMI/track_ahead_free.cpp TrainSubsystems/power.cpp TrainSubsystems/brake.cpp
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
if (ETCS_VENDORED)
    target_include_directories(evc PRIVATE ../libs)
endif()
target_link_libraries(evc PRIVATE orts)

if(WIN32)
    target_link_libraries(evc PRIVATE imagehlp wsock32)
endif()
if(ANDROID)
    target_link_libraries(evc PRIVATE log)
endif()