all: DMI/dmi EVC/evc
DMI/dmi:
	+make -C DMI all
EVC/evc:
	+make -C EVC all
