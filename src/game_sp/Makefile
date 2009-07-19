# =============================================================================
# Rocks'n'Diamonds Makefile (game_sp)
# -----------------------------------------------------------------------------
# (c) 1995-2009 Holger Schemel <info@artsoft.org>
# -----------------------------------------------------------------------------
# MegaPlex version 0.5 beta release xmas 2001 by Frank Schindler,
# based on the Speed Fix 6.3+ by Herman Perk,
# based on original Supaplex by Michael Stopp & Philip Jespersen.
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS =	init.c			\
	main.c			\
	vb_lib.c		\
	vb_vars.c		\
				\
	ASM.c			\
	BitMapObject.c		\
	BugsTerminals.c		\
	Capture.c		\
	DDScrollBuffer.c	\
	DDSpriteBuffer.c	\
	Demo.c			\
	DemoBufferObject.c	\
	DirectDrawGlobals.c	\
	DirectXGlobals.c	\
	Display.c		\
	DoGameStuff.c		\
	Electrons.c		\
	ErrorReporting.c	\
	Explosions.c		\
	FakeDeclares.c		\
	FancyRestore.c		\
	GeneralTricks.c		\
	Globals.c		\
	Infotrons.c		\
	InitGameConditions.c	\
	Input.c			\
	LevelSetPreviewForm.c	\
	MainForm.c		\
	MainGameLoop.c		\
	Marker.c		\
	Murphy.c		\
	OrangeDisk.c		\
	PathTools.c		\
	SettingsObject.c	\
	SnikSnaks.c		\
	Sound.c			\
	TickCountObject.c	\
	TopMost.c		\
	Zonk.c			\
	modAnimations.c		\
	modGeneralTricks.c	\
	modMPX.c

OBJS =	init.o			\
	main.o			\
	vb_lib.o		\
	vb_vars.o		\
				\
	ASM.o			\
	BitMapObject.o		\
	BugsTerminals.o		\
	Capture.o		\
	DDScrollBuffer.o	\
	DDSpriteBuffer.o	\
	Demo.o			\
	DemoBufferObject.o	\
	DirectDrawGlobals.o	\
	DirectXGlobals.o	\
	Display.o		\
	DoGameStuff.o		\
	Electrons.o		\
	ErrorReporting.o	\
	Explosions.o		\
	FakeDeclares.o		\
	FancyRestore.o		\
	GeneralTricks.o		\
	Globals.o		\
	Infotrons.o		\
	InitGameConditions.o	\
	Input.o			\
	LevelSetPreviewForm.o	\
	MainForm.o		\
	MainGameLoop.o		\
	Marker.o		\
	Murphy.o		\
	OrangeDisk.o		\
	PathTools.o		\
	SettingsObject.o	\
	SnikSnaks.o		\
	Sound.o			\
	TickCountObject.o	\
	TopMost.o		\
	Zonk.o			\
	modAnimations.o		\
	modGeneralTricks.o	\
	modMPX.o

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
