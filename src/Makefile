#=============================================================================#
# Makefile for Rocks'n'Diamonds 1.2                                           #
# (c) 1995-98 Holger Schemel, aeglos@valinor.owl.de                           #
#=============================================================================#

ifndef PLATFORM			# platform not defined -- try auto detection
ifdef COMSPEC
PLATFORM = dos
else
PLATFORM = unix
endif
endif

ifdef X11_PATH			# path to X11 specified by top level Makefile
XINC_PATH = $(X11_PATH)/include
XLIB_PATH = $(X11_PATH)/lib
X11_INCL = -I$(XINC_PATH)
X11_LIBS = -L$(XLIB_PATH)
endif

ifndef GAME_DIR			# path to game data not defined -- try '.'
GAME_DIR = .
endif

ifndef SCORE_ENTRIES		# number of score entries per player undefined
SCORE_ENTRIES = MANY_PER_NAME
endif


# The Xpm library is no longer needed to build this program,
# but is used to load graphics if XPM_INCLUDE_FILE is defined.
# If you want to use the Xpm library, convert the PCX files to XPM
# files (and you need corresponding mask files in X11 Bitmap format).

# XPM_INCLUDE_FILE = -DXPM_INCLUDE_FILE="<X11/xpm.h>"
# EXTRA_X11_LIBS = -lXpm


ifeq ($(PLATFORM),dos)		# DOS / Windows

RM = del
PROGNAME = ..\rocks.exe
LIBS = -lm -lalleg

else				# Unix

RM = rm -f
PROGNAME = ../rocksndiamonds

ifeq ($(PLATFORM),solaris)
EXTRA_LIBS = -lnsl -lsocket -R$(XLIB_PATH)
endif

INCL = $(X11_INCL)
LIBS = $(X11_LIBS) $(EXTRA_X11_LIBS) -lX11 -lm $(EXTRA_LIBS)

endif


CONFIG_GAME_DIR = -DGAME_DIR="\"$(GAME_DIR)\""
CONFIG_SCORE_ENTRIES = -D$(SCORE_ENTRIES)

CONFIG = $(CONFIG_GAME_DIR) $(SOUNDS) $(JOYSTICK)	\
	 $(CONFIG_SCORE_ENTRIES) $(XPM_INCLUDE_FILE)

# OPTIONS = -DDEBUG -g -Wall	# only for debugging purposes
OPTIONS = -O3

# SYSTEM = -DSYSV -Ae		# maybe needed for HP-UX

CFLAGS = $(OPTIONS) $(SYSTEM) $(INCL) $(CONFIG)

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
