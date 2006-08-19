# =============================================================================
# Rocks'n'Diamonds Makefile (libgame)
# -----------------------------------------------------------------------------
# (c) 1995-2006 Holger Schemel <info@artsoft.org>
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS =	system.c	\
	gadgets.c	\
	text.c		\
	sound.c		\
	joystick.c	\
	toons.c		\
	pcx.c		\
	image.c		\
	random.c	\
	hash.c		\
	setup.c		\
	misc.c		\
	msdos.c		\
	x11.c		\
	sdl.c

OBJS =	system.o	\
	gadgets.o	\
	text.o		\
	sound.o		\
	joystick.o	\
	toons.o		\
	pcx.o		\
	image.o		\
	random.o	\
	hash.o		\
	setup.o		\
	misc.o		\
	msdos.o		\
	x11.o		\
	sdl.o

LIBGAME = libgame.a


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: $(LIBGAME)

$(LIBGAME): $(OBJS)
	$(AR) cru $(LIBGAME) $(OBJS)
	$(RANLIB) $(LIBGAME)

.c.o:
	$(CC) $(PROFILING) $(CFLAGS) -c $*.c

clean:
	$(RM) $(OBJS)
	$(RM) $(LIBGAME)


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
