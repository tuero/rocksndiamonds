#=============================================================================#
# Makefile for Rocks'n'Diamonds 1.2                                           #
# (c) 1995-98 Holger Schemel, aeglos@valinor.owl.de                           #
#=============================================================================#

#-----------------------------------------------------------------------------#
# configuration section                                                       #
#-----------------------------------------------------------------------------#

# change this to your favorite ANSI C compiler
CC = gcc

# on Solaris and similar systems, you'll need to uncomment this
# EXTRA_LIBS = -lnsl -lsocket 

# specify X11 library path on your system
XLIB_PATH = /usr/X11/lib

# change this to the directory where you want to install game data like levels
GAME_DIR = .

# uncomment this if your system has no joystick include file
# JOYSTICK = -DNO_JOYSTICK

# uncomment this if your system has no sound
# SOUNDS = -DNO_SOUNDS

# choose if you want to allow many global score file entries for one player
# when installing the game in a multi user environment, choose this
# SCORE_ENTRIES = ONE_PER_NAME
# when installing the game in a single user environment, choose this
SCORE_ENTRIES = MANY_PER_NAME

# The XPM-Library is no longer needed to build this program,
# but is used to load graphics if XPM_INCLUDE_FILE is defined,
# because the GIF loading routines are still a bit beta.
# If you want to use the Xpm library, convert the GIF files to Xpm
# files (and you need corresponding mask files in xbm format).

# XPM_INCLUDE_FILE = -DXPM_INCLUDE_FILE="<X11/xpm.h>"
# EXTRA_X11_LIBS = -lXpm

#-----------------------------------------------------------------------------#
# you shouldn't need to change anything below                                 #
#-----------------------------------------------------------------------------#

ifdef COMSPEC
PLATFORM = dos
else
PLATFORM = unix
endif

ifeq ($(PLATFORM),unix)
RM = rm -f
PROGNAME = ../rocksndiamonds
LIBS = -L$(XLIB_PATH) $(EXTRA_X11_LIBS) -lX11 -lm $(EXTRA_LIBS)
else
RM = del
PROGNAME = ..\rocks.exe
LIBS = -lm -lalleg
endif

CPP = $(CC) -E

CONFIG_GAME_DIR = -DGAME_DIR="\"$(GAME_DIR)\""
CONFIG_SCORE_ENTRIES = -D$(SCORE_ENTRIES)

CONFIG = $(CONFIG_GAME_DIR) $(SOUNDS) $(JOYSTICK)	\
	 $(CONFIG_SCORE_ENTRIES) $(XPM_INCLUDE_FILE)

# DEBUG = -DDEBUG -g -Wall -ansi -pedantic
DEBUG = -DDEBUG -g -Wall
# DEBUG = -O3 -Wall -ansi -pedantic
# DEBUG = -O3 -Wall
# DEBUG = -O3

# SYSTEM = -Aa -D_HPUX_SOURCE -Dhpux	# for HP-UX (obsolete)
# SYSTEM = -DSYSV -Ae			# for HP-UX
# INCL = -I/usr/include/X11R5		# for HP-UX and others
# INCL = -I/usr/local/X11/include	# for SunOS and others
# LIBS = -L/usr/lib/X11R5 -lXpm -lX11 -lm
#					# for HP-UX and others
# LIBS = -L/usr/local/X11/lib -lXpm -lX11 -lm -lsocket -R/usr/local/X11/lib
#					# for SunOS and others

# LIBS = -L/usr/X11R6/lib -lXpm -lX11 -lm
# LIBS = -L/usr/X11R6/lib -lX11 -lm

# LIBS = -L/usr/X11R6/lib $(EXTRA_X11_LIBS) -lX11 -lm $(EXTRA_LIBS)

# LIBS = -L$(XLIB_PATH) $(EXTRA_X11_LIBS) -lX11 -lm $(EXTRA_LIBS)

# CFLAGS = -O2 $(CONFIG) $(SYSTEM)
CFLAGS = $(DEBUG) $(CONFIG) $(SYSTEM) $(INCL)

SRCS =	main.c		\
	init.c		\
	events.c	\
	tools.c		\
	screens.c	\
	misc.c		\
	game.c		\
	editor.c	\
	buttons.c	\
	files.c		\
	tape.c		\
	sound.c		\
	joystick.c	\
	cartoons.c	\
	random.c	\
	pcx.c		\
	image.c		\
	network.c	\
	netserv.c	\
	msdos.c

OBJS =	main.o		\
	init.o		\
	events.o	\
	tools.o		\
	screens.o	\
	misc.o		\
	game.o		\
	editor.o	\
	buttons.o	\
	files.o		\
	tape.o		\
	sound.o		\
	joystick.o	\
	cartoons.o	\
	random.o	\
	pcx.o		\
	image.o		\
	network.o	\
	netserv.o	\
	msdos.o

all:	$(PROGNAME)

$(PROGNAME):	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(PROGNAME)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

clean:
	$(RM) *.o
	$(RM) $(PROGNAME)

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
