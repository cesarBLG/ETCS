all: dmi evc
dmi:
	+make -C DMI all
evc:
	+make -C EVC all
