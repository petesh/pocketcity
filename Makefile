
.PHONY: all linux palm clean stuff

stuff:
	@echo Use \`make\` with either \"linux\", \"palm\" or \"all\" as an argument

all: linux palm

linux:
	@echo Making executable for linux-gtk
	cd linux-gtk && $(MAKE) all
	@echo --------------------------
	@echo The executable is now in
	@echo the linux-gtk subdir.
	@echo --------------------------

palm:
	@echo Making executables for PalmOS
	cd palm && $(MAKE) all
	@echo --------------------------
	@echo The .prc files is now in
	@echo the palm/ subdir. Copy
	@echo one of these to your Palm.
	@echo --------------------------

clean:
	cd linux-gtk && $(MAKE) clean
	cd palm && $(MAKE) clean

