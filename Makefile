
VERSION := $(shell grep Version CHANGELOG | head -1 | awk '{print $$2}')

nearlylive: VERSION = $(shell date +%Y.%m.%d)

.PHONY: all linux palm clean stuff docs

stuff:
	@echo Use \`make\` with either \"linux\", \"palm\" or \"all\" as an argument
#all-graphic
all: all-linux-gtk all-palm

all-%:
	cd $(@:all-%=%) && $(MAKE) all

debug: debug-linux-gtk debug-palm

debug-%:
	cd $(@:debug-%=%) && $(MAKE) debug

linux: all-linux

palm: all-palm

#clean-graphic
clean: clean-linux-gtk clean-palm

clean-%:
	cd $(@:clean-%=%) && $(MAKE) clean

SVFILE=pcity-source-$(VERSION)

nearlylive dist:
	cd ..; \
	ln -s pcity $(SVFILE); \
	tar chf - $(SVFILE) | gzip -c > $(SVFILE).tgz; \
	rm $(SVFILE)

docs:
	doxygen
	@if [ -s Doxy.warn ]; then\
		echo "Warnings in Doxy.warn"; \
		exit 1; \
	fi
