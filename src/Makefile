#
# Makefile fuer "Rocks'n'Diamonds -- McDuffin Strikes Back"
#

PROGNAME = rocksndiamonds

RM = rm -f
CC = gcc
CPP = $(CC) -E
# CC = cc				# for HP-UX and others

GAME_DIR = -DGAME_DIR=\".\"		# path of the game and its data
# JOYSTICK = -DNO_JOYSTICK		# no joystick
# SOUNDS = -DNO_SOUNDS			# no sounds
# SCORE_ENTRIES = -DONE_PER_NAME	# only one score entry per name
SCORE_ENTRIES = -DMANY_PER_NAME		# many score entries per name

# the XPM-Library is needed to build this program:
XPM_INCLUDE_FILE = -DXPM_INCLUDE_FILE="<X11/xpm.h>"

CONFIG = $(GAME_DIR) $(SOUNDS) $(JOYSTICK)	\
	 $(SCORE_ENTRIES) $(XPM_INCLUDE_FILE)

# DEBUG = -DDEBUG -g -ansi -pedantic -Wall
# DEBUG = -DDEBUG -g -Wall
DEBUG = -O6

# SYSTEM = -Aa -D_HPUX_SOURCE -Dhpux	# for HP-UX (obsolete)
# SYSTEM = -DSYSV -Ae			# for HP-UX
# SYSTEM = -DSYSV			# for systems without 'usleep()'
# INCL = -I/usr/include/X11R5		# for HP-UX and others
# LIBS = -lXpm -lX11 -lm
# LIBS = -L/usr/lib/X11R5 -lXpm -lX11 -lm # for HP-UX and others
LIBS = -lXpm -lXpm -lXpm -lX11 -lm	# triple -lXpm; else I got an error...

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
	sound.c

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
	sound.o

all:	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(PROGNAME)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

clean:
	$(RM) $(OBJS)

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
