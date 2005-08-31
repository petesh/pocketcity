How to build pocketcity

You need the following components:-

General components:
	- gnu make
		- On any linux system, use cygwin on windows
	- imagemagick (for converting the picture files)
		http://www.imagemagick.org/

Palm files:
	- pilrc (2.9 patch 10 minimum, 3 preferred!)
		http://sourceforge.net/projects/pilrc/
	- prc tools
		http://prc-tools.sourceforge.net/
	- palm development SDK
		http://www.palmsource.com/
	- Sony palm SDK (for sony high resolution)
		http://www.us.sonypdadev.com/top.html
		[ note: needs to be converted to support GCC ]
		http://yakko.cs.wmich.edu/~rattles/development/clie/
		I believe sony in their infinite wisdom have discontinued the
		us sonypdadev site so this may be a moot point. I'll probably
		bundle a copy of the SDK for people to use
	- Palm Specific SDK
		- 5-way navigator support

	- Palm Emulator (and appropriate ROMs)
	- Palm Simulator.
		http://www.palmsource.com/

		[ If you're having problems with getting any of the SDKs then
		  contact me and I'll see what I can sort out ]

Linux:
	- gtk 2.0
		http://www.gtk.org/

Windows/Cygwin:
	- cygwin (reasonably modern)
		http://www.cygwin.com/
		Must have gtk2


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

Build information:
You need to build the graphic directory first. This is because there is no
reference for rebuilding these files from within the palm and linux build tree.
to do this perform a make in this directory.

Building linux:
I'm not using autoconf or any of those ism's I should really, but I've neither
the time nor the inclination to learn it, so what I've been doing is building
it on:
	1. Solaris 10
	2. Mandrake 9
	3. Cygwin
I'll try a few other distros (debian, fedora), and try to make sure that it
builds on at least most of these. I'm primarily building the linux release on
Solaris, this is due to it being the primary operating system I've got on all
my systems. I don't want any complaints about this, I just have more experience
with Solaris and it keeps me in beer and clothes!

Building palm:
cd to the palm directory. There are 3 primary targets: all, debug and logging.
All builds the code in a non-debug format, placing the executable in out-<>
directories (e.g. out-standard, out-palm5, out-sony), and the .pdb files in
the current diretory.
debug does out-debug, etc for the same targets,
logging does out-standard, etc for the same targets.
By default a debug target is built.

Differences between logging and debug levels:
	debug only includes the -g option in the build, you can attach to it
	using the palm debugger.

	logging also places information about the progress of the game into a
	logfile on the machine that is running the simulator/emulator.
	the logfiles are put in '/tmp' currently.
	The extra code is not needed if you're testing on an real palm via
	serial debugging.


