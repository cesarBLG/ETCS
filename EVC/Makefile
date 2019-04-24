OBJECTS = evc.o bitreader.o variable.o packet.o MA/movement_authority.o DMI/dmi.o
all: evc
evc: $(OBJECTS)
	g++ $(OBJECTS) -o evc -pthread
%.o: %.cpp
	g++ -c $< -o $@
