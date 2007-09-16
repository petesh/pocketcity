# Create the montage image of the individual icons.
# requires imagemagick in the path (montage)
#
include ../../Makefile-settings

MONTAGE=montage

TYPES=bw color

TARGETS=speed

DIRS=icons icons-color
RELEASEDIRS=$(DIRS:%=release-%)
DEBUGDIRS=$(DIRS:%=debug-%)
CLEANDIRS=$(DIRS:%=clean-%)

debug: $(DEBUGDIRS) $(TYPES)
release: $(RELEASEDIRS) $(TYPES)

all: $(BUILD_MODE)

release-%:
	cd $(@:release-%=%) && $(MAKE) release
debug-%:
	cd $(@:debug-%=%) && $(MAKE) debug
clean-%:
	cd $(@:clean-%=%) && $(MAKE) clean

.PHONY: $(TYPES)

$(TYPES):
	$(MAKE) TARGET=$@ $(TARGETS:%=%-$@.bmp)

ALLTARGETS=$(TARGETS:%=%-$(TARGET).bmp)
SPEEDFILES=paused slow normal fast ultra
SPEEDBMPS=$(SPEEDFILES:%=speed_%-$(TARGET).bmp)

$(ALLTARGETS): $(SPEEDBMPS)
	$(MONTAGE) -geometry 10x10+0+0 -tile 10x1 -adjoin $(SPEEDBMPS) $@

speed-grey.bmp:

CLEANTARGETS=$(TARGETS:%=%-*.bmp)

clean: $(CLEANDIRS)
	$(RM) $(CLEANTARGETS)
