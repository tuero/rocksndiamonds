#
# Makefile fuer "Rocks'n'Diamonds -- McDuffin Strikes Back"
#

PROGNAME = rocksndiamonds
SERVNAME = rnd_server

RM = rm -f
CC = gcc
CPP = $(CC) -E
# CC = cc				# for HP-UX and others

GAME_DIR = -DGAME_DIR=\".\"		# path of the game and its data
# JOYSTICK = -DNO_JOYSTICK		# no joystick
# SOUNDS = -DNO_SOUNDS			# no sounds
# SCORE_ENTRIES = -DONE_PER_NAME	# only one score entry per name
SCORE_ENTRIES = -DMANY_PER_NAME		# many score entries per name

# The XPM-Library is no longer needed to build this program,
# but is used to load graphics if XPM_INCLUDE_FILE is defined,
# because the GIF loading routines are still a bit beta.
# If you use the Xpm library, convert the GIF files to Xpm
# files (and the mask files ('*Maske.gif') to xbm files).

# XPM_INCLUDE_FILE = -DXPM_INCLUDE_FILE="<X11/xpm.h>"

CONFIG = $(GAME_DIR) $(SOUNDS) $(JOYSTICK)	\
	 $(SCORE_ENTRIES) $(XPM_INCLUDE_FILE)

# DEBUG = -DDEBUG -g -ansi -pedantic -Wall
DEBUG = -DDEBUG -g -Wall
# DEBUG = -O6

# SYSTEM = -Aa -D_HPUX_SOURCE -Dhpux	# for HP-UX (obsolete)
# SYSTEM = -DSYSV -Ae			# for HP-UX
# INCL = -I/usr/include/X11R5		# for HP-UX and others
# INCL = -I/usr/local/X11/include	# for SunOS and others
# LIBS = -L/usr/lib/X11R5 -lXpm -lX11 -lm
#					# for HP-UX and others
# LIBS = -L/usr/local/X11/lib -lXpm -lX11 -lm -lsocket -R/usr/local/X11/lib
#					# for SunOS and others

# LIBS = -L/usr/X11R6/lib -lXpm -lX11 -lm
LIBS = -L/usr/X11R6/lib -lX11 -lm

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
	gfxload.c	\
	gifload.c	\
	gif.c		\
	send.c		\
	new.c		\
	network.c

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
	gfxload.o	\
	gifload.o	\
	gif.o		\
	send.o		\
	new.o		\
	network.o

all:	$(PROGNAME) $(SERVNAME)

$(PROGNAME):	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(PROGNAME)

$(SERVNAME):	$(SERVNAME).c
	$(CC) $(CFLAGS) $(SERVNAME).c $(LIBS) -o $(SERVNAME)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

clean:
	$(RM) $(PROGNAME) $(SERVNAME) $(OBJS)

backup:
	./make_backup.sh

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
