OBJECTS = evc.o DMI/dmi.o Supervision/national_values.o Supervision/fixed_values.o Supervision/conversion_model.o \
Supervision/curve_calc.o Supervision/distance.o \
Supervision/speed_profile.o Supervision/supervision.o Supervision/targets.o Supervision/train_data.o \
OR_interface/interface.o
A = packet.o bitreader.o variable.o MA/movement_authority.o
all: evc
evc: $(OBJECTS)
	g++ $(OBJECTS) -o evc -pthread -g -rdynamic -lorts
%.o: %.cpp
	g++ -c $< -o $@ -g -rdynamic
