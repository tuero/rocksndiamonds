#=============================================================================#
# Makefile for Rocks'n'Diamonds 1.4.0                                         #
# (c) 1995-1999 Holger Schemel, aeglos@valinor.owl.de                         #
#=============================================================================#

ifndef PLATFORM			# platform not specified -- try auto detection
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


ifeq ($(PLATFORM),dos)		# DOS / Windows

RM = del
PROGNAME = ../rocks.exe
LIBS = -s -lm -lalleg

else				# Unix

RM = rm -f
PROGNAME = ../rocksndiamonds

ifeq ($(PLATFORM),solaris)
EXTRA_LIBS = -lnsl -lsocket -R$(XLIB_PATH)
endif

USE_SDL = true
# USE_SDL = false
# USE_SDL_OLD_LIBS = true
USE_SDL_OLD_LIBS = false

ifeq ($(USE_SDL_OLD_LIBS),true)
SDL_EXTRA_LIBS = -lIMG -lmixer
else
SDL_EXTRA_LIBS = -lSDL_image -lSDL_mixer
endif

SDL_CFLAGS := -DUSE_SDL_LIBRARY $(shell sdl-config --cflags)
SDL_LDFLAGS := $(SDL_EXTRA_LIBS) $(shell sdl-config --libs)

X11_CFLAGS = $(X11_INCL)
X11_LDFLAGS = $(X11_LIBS) -lX11

ifeq ($(USE_SDL),true)
SYS_CFLAGS  = $(SDL_CFLAGS)
SYS_LDFLAGS = $(SDL_LDFLAGS)
else
SYS_CFLAGS  = $(X11_CFLAGS)
SYS_LDFLAGS = $(X11_LDFLAGS)
endif

INCL = $(SYS_CFLAGS)
LIBS = $(SYS_LDFLAGS) $(EXTRA_LIBS) -lm

endif


ifdef RO_GAME_DIR		# path to read-only game data specified
CONFIG_RO_GAME_DIR = -DRO_GAME_DIR="\"$(RO_GAME_DIR)\""
endif

ifdef RW_GAME_DIR		# path to writable game data specified
CONFIG_RW_GAME_DIR = -DRW_GAME_DIR="\"$(RW_GAME_DIR)\""
endif

ifdef SCORE_ENTRIES		# number of score entries per player specified
CONFIG_SCORE_ENTRIES = -D$(SCORE_ENTRIES)
endif

CONFIG_GAME_DIR = $(CONFIG_RO_GAME_DIR) $(CONFIG_RW_GAME_DIR)


CONFIG = $(CONFIG_GAME_DIR) $(CONFIG_SCORE_ENTRIES) $(SOUNDS) $(JOYSTICK)

# OPTIONS = -DDEBUG -g -Wall -ansi -pedantic	# only for debugging purposes
OPTIONS = -DDEBUG -g -Wall			# only for debugging purposes
# OPTIONS = -O3 -Wall -ansi -pedantic
# OPTIONS = -O3 -Wall
# OPTIONS = -O3

# SYSTEM = -DSYSV -Ae				# may be needed for HP-UX

CFLAGS = $(OPTIONS) $(SYSTEM) $(SYS_CFLAGS) $(CONFIG)

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
	msdos.c		\
	system.c	\
	x11.c		\
	sdl.c

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
	msdos.o		\
	system.o	\
	x11.o		\
	sdl.o

all:	$(PROGNAME)

$(PROGNAME):	$(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(PROGNAME)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

clean:
	$(RM) *.o
	$(RM) $(PROGNAME)
