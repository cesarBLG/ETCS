FILES = evc.o DMI/dmi.o Supervision/national_values.o Supervision/fixed_values.o \
Supervision/curve_calc.o  Supervision/conversion_model.o  Position/distance.o\
Supervision/speed_profile.o Supervision/supervision.o Supervision/targets.o Supervision/train_data.o \
Supervision/acceleration.o antenna.o MA/movement_authority.o MA/mode_profile.o Position/linking.o \
OR_interface/interface.o SSP/ssp.o Packets/packets.o Procedures/mode_transition.o LX/level_crossing.o \
Packets/messages.o Packets/radio.o  Euroradio/session.o Euroradio/terminal.o \
Procedures/start.o Procedures/override.o Procedures/train_trip.o Procedures/level_transition.o \
Procedures/stored_information.o TrackConditions/track_conditions.o Time/clock.o Position/geographical.o \
DMI/text_messages.o DMI/windows.o TrainSubsystems/power.o TrainSubsystems/brake.o \
$(patsubst %.cpp, %.o, $(wildcard NationalFN/*.cpp))
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
OBJECTS = $(addprefix $(BUILD_DIR), $(FILES))
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
