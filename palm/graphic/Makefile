# Create the montage image of the individual icons.
# requires imagemagick in the path (montage)

MONTAGE=montage

TYPES=bw color

TARGETS=speed

DIRS=icons icons-color
ALLDIRS=$(DIRS:%=all-%)
DEBUGDIRS=$(DIRS:%=debug-%)
CLEANDIRS=$(DIRS:%=clean-%)

all: $(ALLDIRS) $(TYPES)
debug: $(DEBUGDIRS) $(TYPES)

all-%:
	cd $(@:all-%=%) && $(MAKE) all
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
