# Create the montage image of the individual icons.
# requires imagemagick in the path (montage)

MONTAGE=montage

all debug: montage.bmp

files=bulldozer.bmp residential.bmp commercial.bmp industrial.bmp \
	road.bmp powerplant.bmp nuclear.bmp powerline.bmp \
	waterplant.bmp waterpipe.bmp tree.bmp water.bmp \
	def_firemen.bmp def_police.bmp def_military.bmp query.bmp extra.bmp \
	coin.bmp people.bmp location.bmp

montage.bmp: $(files)
	$(MONTAGE) -geometry 10x10+0+0 -tile 80x4 -adjoin $(files) $@

clean:
	rm -f montage.bmp
