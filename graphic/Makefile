
ICONFILE=tile
COLORS=bw grey color
DIM=16x16
SHAPE=32x32
MAPFILE=tilemapping.txt
HEADERFILE=tileheader.h
DEPPNGS=`sed -n -e '/^#/d' -e 's/:.*/.png/p' $(MAPFILE)`

ALLFILES=$(COLORS:%=$(ICONFILE)-%)

.PHONY: all-%
.PRECIOUS: $(ALLFILES)

all: $(ALLFILES)

$(ICONFILE)-%: $(DEPPNGS:%=$$*/%) $(MAPFILE)
	./dotilezones.sh -t $* -d $(DIM) -s $(SHAPE) $(MAPFILE) \
		$(ICONFILE)-$(DIM) tileheader.h
	touch $@

clean:
	rm -f $(ALLFILES) *.png *.bmp
