SOURCES = evc.cpp DMI/dmi.cpp Supervision/national_values.cpp Supervision/fixed_values.cpp \
Supervision/curve_calc.cpp  Supervision/conversion_model.cpp  Position/distance.cpp\
Supervision/speed_profile.cpp Supervision/supervision.cpp Supervision/targets.cpp Supervision/train_data.cpp \
Supervision/emergency_stop.cpp \
Supervision/acceleration.cpp antenna.cpp MA/movement_authority.cpp MA/mode_profile.cpp Position/linking.cpp \
OR_interface/interface.cpp SSP/ssp.cpp Packets/packets.cpp Procedures/mode_transition.cpp LX/level_crossing.cpp \
Packets/messages.cpp Packets/radio.cpp Packets/vbc.cpp Euroradio/session.cpp Euroradio/terminal.cpp \
Procedures/start.cpp Procedures/override.cpp Procedures/train_trip.cpp Procedures/level_transition.cpp \
Procedures/stored_information.cpp TrackConditions/track_conditions.cpp Time/clock.cpp Position/geographical.cpp \
DMI/text_messages.cpp DMI/windows.cpp DMI/track_ahead_free.cpp TrainSubsystems/power.cpp TrainSubsystems/brake.cpp \
$(wildcard NationalFN/*.cpp)
LIB = evc.so
ARCHIVE = evc.a
EXE = evc
CXX = g++
CC = gcc
DEBUG=-g -rdynamic -D_GLIBCXX_DEBUG
LDFLAGS = -lorts
CXXFLAGS = -std=c++11
BUILD_DIR = build/linux/
ifeq ($(TARGET),WIN32)
EXE = evc.exe
LIB = evc.dll
CXX = x86_64-w64-mingw32-g++ -m64
CC = x86_64-w64-mingw32-gcc -m64
CFLAGS = -I.
FILES += resource.res
LDFLAGS = -lstdc++ -lwsock32 -L. -Bdynamic -lorts -limagehlp
DEBUG=
WINDRES = x86_64-w64-mingw32-windres
#-D_GLIBCXX_DEBUG
BUILD_DIR = build/win32/
endif
OBJECTS = $(patsubst %.cpp,%.o,$(addprefix $(BUILD_DIR), $(SOURCES)))
all: $(EXE)
$(EXE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(DEBUG) -o evc -pthread  $(LDFLAGS) -Wall
$(BUILD_DIR)%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(DEBUG) -c $< -o $@ $(CFLAGS) -Wall -Wno-sign-compare $(CXXFLAGS) -fpic

$(LIB): $(OBJECTS)
	$(CXX) -shared -o evc.dll $(OBJECTS) $(LDFLAGS) -g
$(ARCHIVE): $(OBJECTS)
	ar rcs

$(BUILD_DIR)resource.res: resource.rc
	$(WINDRES) resource.rc -O coff -o $(BUILD_DIR)resource.res

clean:
	rm $(OBJECTS)
