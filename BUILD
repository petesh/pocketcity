How to build pocketcity

You need the following components:-

General components:
	- gnu make
		- On any linux system, use cygwin on windows
	- imagemagick (for converting the picture files)
		http://www.imagemagick.org/

Palm files:
	- pilrc (2.9 patch 10 minimum)
		http://sourceforge.net/projects/pilrc/
	- prc tools
		http://prc-tools.sourceforge.net/
	- palm development SDK
		http://www.palmsource.com/
	- Sony palm SDK (for sony high resolution)
		http://www.us.sonypdadev.com/top.html
		[ note: needs to be converted to support GCC ]
		http://yakko.cs.wmich.edu/~rattles/development/clie/

	- Palm Emulator (and appropriate ROMs)
	- Palm Simulator.
		http://www.palmsource.com/

Linux:
	- gtk 2.0
		http://www.gtk.org/

Windows/Cygwin:
	- cygwin (reasonably modern)
		http://www.cygwin.com/
	- GTK for windows
		http://web.sfc.keio.ac.jp/~s01397ms/cygwin/


The usual preamble:

I didn't write any of the tools above, so if you're having problems then
while I may be able to help you in the short run, you'll only hurt yourself
if you can't figure it out in the long run.

The Sony SDK files need to have the use of enum for the trap numbers converted
to using #defines, otherwise GCC can't compile the files, complaining about,
if I remember, variables being used for the traps; it's been so long, I just
can't be sure.

When you build under the Windows/cygwin/GTK environment you will probably
encounter an inttypes.h is missing compilation error. Just create the file
and #include <stddef.h>??


