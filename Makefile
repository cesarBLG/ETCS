all: dmi evc
dmi:
	make -C DMI -j5
evc:
	make -C EVC -j5
