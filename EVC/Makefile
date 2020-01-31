OBJECTS = evc.o DMI/dmi.o Supervision/national_values.o Supervision/fixed_values.o \
Supervision/curve_calc.o  Supervision/conversion_model.o  Position/distance.o\
Supervision/speed_profile.o Supervision/supervision.o Supervision/targets.o Supervision/train_data.o \
Supervision/acceleration.o antenna.o MA/movement_authority.o Position/linking.o \
OR_interface/interface.o SSP/ssp.o Packets/packets.o Procedures/mode_transition.o LX/level_crossing.o \
Packets/messages.o
CXX = g++
CC = gcc
DEBUG=-g -rdynamic
LDFLAGS =
CXXFLAGS = -std=c++17
ifeq ($(TARGET),WIN32)
CXX = x86_64-w64-mingw32-g++ -m64
CC = x86_64-w64-mingw32-gcc -m64
CFLAGS = -I.
LDFLAGS += -lstdc++ -lwsock32 -L.
DEBUG=-g
endif
all: evc
evc: $(OBJECTS)
	$(CXX) $(OBJECTS) $(DEBUG) -o evc -pthread  $(LDFLAGS) -lorts -Wall
%.o: %.cpp
	$(CXX) $(DEBUG) -c $< -o $@ $(CFLAGS) -Wall -Wno-sign-compare $(CXXFLAGS)
