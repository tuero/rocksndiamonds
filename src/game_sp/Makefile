# =============================================================================
# Makefile for native Supaplex engine for Rocks'n'Diamonds (game_sp)
# -----------------------------------------------------------------------------
# (c) 1995-2009 Holger Schemel <info@artsoft.org>
# -----------------------------------------------------------------------------
# based on MegaPlex version 0.5 beta release xmas 2001 by Frank Schindler,
# based on the Speed Fix 6.3+ by Herman Perk,
# based on original Supaplex by Michael Stopp & Philip Jespersen
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS =	init.c			\
	file.c			\
	main.c			\
	vb_lib.c		\
				\
	ASM.c			\
	BugsTerminals.c		\
	DDScrollBuffer.c	\
	DDSpriteBuffer.c	\
	Display.c		\
	DoGameStuff.c		\
	Electrons.c		\
	Explosions.c		\
	Globals.c		\
	Infotrons.c		\
	InitGameConditions.c	\
	Input.c			\
	MainForm.c		\
	MainGameLoop.c		\
	Murphy.c		\
	OrangeDisk.c		\
	SnikSnaks.c		\
	Sound.c			\
	Zonk.c

OBJS =	init.o			\
	file.o			\
	main.o			\
	vb_lib.o		\
				\
	ASM.o			\
	BugsTerminals.o		\
	DDScrollBuffer.o	\
	DDSpriteBuffer.o	\
	Display.o		\
	DoGameStuff.o		\
	Electrons.o		\
	Explosions.o		\
	Globals.o		\
	Infotrons.o		\
	InitGameConditions.o	\
	Input.o			\
	MainForm.o		\
	MainGameLoop.o		\
	Murphy.o		\
	OrangeDisk.o		\
	SnikSnaks.o		\
	Sound.o			\
	Zonk.o

GAME_SP = game_sp.a


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: $(GAME_SP)

$(GAME_SP): $(OBJS)
	$(AR) cru $(GAME_SP) $(OBJS)
	$(RANLIB) $(GAME_SP)

.c.o:
	$(CC) $(PROFILING) $(CFLAGS) -c $*.c

clean:
	$(RM) $(OBJS)
	$(RM) $(GAME_SP)


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
