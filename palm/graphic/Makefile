# Create the montage image of the individual icons.
# requires imagemagick in the path (montage)

MONTAGE=montage

TYPES=bw color

TARGETS=speed

all: $(TYPES)

.PHONY: $(TYPES)

$(TYPES):
	$(MAKE) $(MAKEFLAGS) TARGET=$@ $(TARGETS:%=%-$@.bmp)

ALLTARGETS=$(TARGETS:%=%-$(TARGET).bmp)
SPEEDFILES=paused slow normal fast ultra
SPEEDBMPS=$(SPEEDFILES:%=speed_%-$(TARGET).bmp)

$(ALLTARGETS): $(SPEEDBMPS)
	$(MONTAGE) -geometry 10x10+0+0 -tile 10x1 -adjoin $(SPEEDBMPS) $@

speed-grey.bmp:

CLEANTARGETS=$(TARGETS:%=%-*.bmp)

clean:
	$(RM) $(CLEANTARGETS)
