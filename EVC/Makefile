OBJECTS = evc.o bitreader.o variable.o packet.o MA/movement_authority.o
all: evc
evc: $(OBJECTS)
	g++ $(OBJECTS) -o evc
%.o: %.cpp
	g++ -c $< -o $@
