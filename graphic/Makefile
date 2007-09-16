
# $Id$

ICONFILE=tile
COLORS=color grey bw
OSIZE=16
DIM=$(OSIZE)x$(OSIZE)
MSIZE=4
MDIM=$(MSIZE)x$(MSIZE)
TILEWIDTH=32
MAPFILE=tilemapping.txt
HEADERFILE=tileheader.h
DEPPNGS=`sed -ne '/^#/d' -e 's/:.*/.png/p' $(HEADERFILE)`

ALLFILES=$(COLORS:%=$(ICONFILE)-$(DIM)-%.png)

.PHONY: all-%
.PRECIOUS: $(ALLFILES)

all: $(ALLFILES)

$(ICONFILE)-$(DIM)-%.png: $(MAPFILE)
	./dotilezones.sh -t $* -d $(DIM) -s $(TILEWIDTH) -c welem_t \
		$(MAPFILE) $(ICONFILE)-$(DIM) $(HEADERFILE)
	eval `identify -format "w=%w;h=%h" $(ICONFILE)-$(DIM)-$*.bmp`; \
	w=`expr $$w / $(MSIZE)`; \
	h=`expr $$h / $(MSIZE)`; \
	convert $(ICONFILE)-$(DIM)-$*.bmp -resize $${w}x$${h} \
	    $(ICONFILE)-$(MDIM)-$*.bmp; \
	convert $(ICONFILE)-$(DIM)-$*.png -resize $${w}x$${h} \
	    $(ICONFILE)-$(MDIM)-$*.png

clean:
	rm -f $(ALLFILES) *.png *.bmp *.miff $(HEADERFILE)
