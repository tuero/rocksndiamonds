#=============================================================================#
# Makefile for Artsoft Retro-Game Library                                     #
# (c) 1994-2000 Holger Schemel, info@artsoft.org                              #
#=============================================================================#

SRCS =	system.c	\
	gadgets.c	\
	text.c		\
	sound.c		\
	pcx.c		\
	image.c		\
	random.c	\
	misc.c		\
	msdos.c		\
	x11.c		\
	sdl.c

OBJS =	system.o	\
	gadgets.o	\
	text.o		\
	sound.o		\
	pcx.o		\
	image.o		\
	random.o	\
	misc.o		\
	msdos.o		\
	x11.o		\
	sdl.o

LIBGAME = libgame.a


all: $(LIBGAME)

$(LIBGAME): $(OBJS)
	$(AR) cru $(LIBGAME) $(OBJS)
	$(RANLIB) $(LIBGAME)

.c.o:
	$(CC) $(PROFILING) $(CFLAGS) -c $*.c

clean:
	$(RM) $(OBJS)
	$(RM) $(LIBGAME)


#-----------------------------------------------------------------------------#
# development only stuff                                                      #
#-----------------------------------------------------------------------------#

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
