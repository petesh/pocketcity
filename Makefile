
VERSION := $(shell grep Version CHANGELOG | head -1 | awk '{print $$2}')

nearlylive: VERSION = $(shell date +%Y.%m.%d)

.PHONY: all linux palm clean stuff

stuff:
	@echo Use \`make\` with either \"linux\", \"palm\" or \"all\" as an argument

all: all-graphic all-linux-gtk all-palm

all-%:
	cd $(@:all-%=%) && $(MAKE) $(MAKEFLAGS) all

debug: debug-linux-gtk debug-palm

debug-%:
	cd $(@:debug-%=%) && $(MAKE) $(MAKEFLAGS) debug

linux: all-linux

palm: all-palm

clean: clean-graphic clean-linux-gtk clean-palm

clean-%:
	cd $(@:clean-%=%) && $(MAKE) $(MAKEFLAGS) clean

SVFILE=pcity-source-$(VERSION)

nearlylive dist:
	cd ..; \
	ln -s pcity $(SVFILE); \
	tar chf - $(SVFILE) | gzip -c > $(SVFILE).tgz; \
	rm $(SVFILE)
