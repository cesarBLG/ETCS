LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := evc

LOCAL_C_FLAGS +=

LOCAL_CPP_FEATURES := rtti

# Add your application source files here...
LOCAL_SRC_FILES := evc.cpp DMI/dmi.cpp DMI/windows.cpp Supervision/national_values.cpp Supervision/fixed_values.cpp \
                   Supervision/curve_calc.cpp  Supervision/conversion_model.cpp  Position/distance.cpp\
                   Supervision/speed_profile.cpp Supervision/supervision.cpp Supervision/targets.cpp Supervision/train_data.cpp \
                   Supervision/acceleration.cpp antenna.cpp MA/movement_authority.cpp MA/mode_profile.cpp Position/linking.cpp \
                   OR_interface/interface.cpp SSP/ssp.cpp Packets/packets.cpp Procedures/mode_transition.cpp LX/level_crossing.cpp \
                   Packets/messages.cpp Packets/radio.cpp Euroradio/session.cpp Euroradio/terminal.cpp Procedures/override.cpp Procedures/train_trip.cpp Procedures/level_transition.cpp \
                   Procedures/stored_information.cpp Procedures/start.cpp TrackConditions/track_conditions.cpp Time/clock.cpp Position/geographical.cpp \
                   DMI/text_messages.cpp TrainSubsystems/power.cpp TrainSubsystems/brake.cpp \
                   NationalFN/nationalfn.cpp NationalFN/asfa.cpp \
                   $(wildcard NationalFN/*.cpp)

LOCAL_SHARED_LIBRARIES := orts

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
