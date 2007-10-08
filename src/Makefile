# =============================================================================
# Rocks'n'Diamonds Makefile (src)
# -----------------------------------------------------------------------------
# (c) 1995-2006 Holger Schemel <info@artsoft.org>
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

.EXPORT_ALL_VARIABLES:

ifndef PLATFORM				# unknown platform -- try auto detection
ifdef COMSPEC
PLATFORM = msdos
else
PLATFORM = unix
endif
endif

ifdef X11_PATH				# path to X11 from top level Makefile
XINC_PATH = $(X11_PATH)/include
XLIB_PATH = $(X11_PATH)/lib
X11_INCL = -I$(XINC_PATH)
X11_LIBS = -L$(XLIB_PATH) -lX11
endif

AR = ar
RANLIB = ranlib
ETAGS = etags
RM = rm -f

BMP2ICO = bmp2ico
WINDRES = windres

PROGBASE = rocksndiamonds

ifdef SPECIAL_EDITION			# create special edition
PROGBASE = $(SPECIAL_EDITION)
endif

PROGNAME = ../$(PROGBASE)


# -----------------------------------------------------------------------------
# configuring platform
# -----------------------------------------------------------------------------

ifeq ($(PLATFORM),solaris)		# native compiling to Solaris
EXTRA_LDFLAGS = -lnsl -lsocket -R$(XLIB_PATH)
PLATFORM = unix
endif

ifeq ($(PLATFORM),macosx)		# native compiling to Mac OS X
EXTRA_LDFLAGS = -lstdc++
PLATFORM = unix
TARGET = sdl
endif

ifeq ($(PLATFORM),os2)			# native compiling to OS/2
RANLIB = echo
PROGNAME = ../$(PROGBASE).exe
EXTRA_CFLAGS = -Zbin-files -D__ST_MT_ERRNO__ -Zmtd -fomit-frame-pointer
EXTRA_LDFLAGS = -lsocket
PLATFORM = unix
TARGET = x11
endif

ifeq ($(PLATFORM),unix)			# native compiling to Unix (generic)
PROFILING_FLAGS = -pg
endif

ifeq ($(PLATFORM),cross-win32)		# cross-compiling to Windows
PROGNAME = ../$(PROGBASE).exe
EXTRA_LDFLAGS = -lshfolder -lwsock32
TARGET = sdl
endif

ifeq ($(PLATFORM),cross-msdos)		# cross-compiling to MS-DOS
PROGBASE = rocks
PROGNAME = ../$(PROGBASE).exe
TARGET = allegro
endif

ifeq ($(PLATFORM),msdos)		# native compiling to MS-DOS
PROGBASE = rocks
PROGNAME = ../$(PROGBASE).exe
SYS_CFLAGS = -DTARGET_X11
SYS_LDFLAGS = -s -lalleg
RM = del
endif


# -----------------------------------------------------------------------------
# configuring target
# -----------------------------------------------------------------------------

ifeq ($(TARGET),x11)			# compiling for X11 target
SYS_CFLAGS  = -DTARGET_X11 $(X11_INCL)
SYS_LDFLAGS = $(X11_LIBS)
endif

ifeq ($(TARGET),sdl)			# compiling for SDL target
SYS_CFLAGS  = -DTARGET_SDL $(shell sdl-config --cflags)
SDL_LIBS = -lSDL_image -lSDL_mixer -lSDL_net -lsmpeg
SYS_LDFLAGS = $(SDL_LIBS) $(shell sdl-config --libs)
endif

ifeq ($(TARGET),sdl-static)		# compiling for SDL target (static)
SYS_CFLAGS  = -DTARGET_SDL $(shell sdl-config --cflags)
SDL_LIBS = -lSDL_image -lSDL_mixer -lSDL_net -lsmpeg
SYS_LDFLAGS = $(SDL_LIBS) $(shell sdl-config --static-libs)
endif

ifeq ($(TARGET),allegro)		# compiling for Allegro target
SYS_CFLAGS = -DTARGET_ALLEGRO -I$(CROSS_PATH_MSDOS)/include
SYS_LDFLAGS = -lalleg
endif


# -----------------------------------------------------------------------------
# configuring compile-time definitions
# -----------------------------------------------------------------------------

ifdef RO_GAME_DIR			# path to read-only game data specified
CONFIG_RO_GAME_DIR = -DRO_GAME_DIR="\"$(RO_GAME_DIR)\""
endif

ifdef RW_GAME_DIR			# path to writable game data specified
CONFIG_RW_GAME_DIR = -DRW_GAME_DIR="\"$(RW_GAME_DIR)\""
endif

ifdef SCORE_ENTRIES			# number of score entries per player
CONFIG_SCORE_ENTRIES = -D$(SCORE_ENTRIES)
endif

ifdef SPECIAL_EDITION
ifeq ($(SPECIAL_EDITION),rnd_jue)	# create special edition
CONFIG_SPECIAL = -DCREATE_SPECIAL_EDITION -DCREATE_SPECIAL_EDITION_RND_JUE
endif
endif

CONFIG_GAME_DIR = $(CONFIG_RO_GAME_DIR) $(CONFIG_RW_GAME_DIR)
CONFIG_GAME = $(CONFIG_GAME_DIR) $(CONFIG_SCORE_ENTRIES) $(CONFIG_SPECIAL)

CONFIG = $(CONFIG_GAME) $(JOYSTICK)
# DEBUG = -DDEBUG -g
# PROFILING = $(PROFILING_FLAGS)

# OPTIONS = $(DEBUG) -Wall			# only for debugging purposes
# OPTIONS = $(DEBUG) -O3 -Wall			# only for debugging purposes
OPTIONS = $(DEBUG) -Wall			# only for debugging purposes
# OPTIONS = $(DEBUG) -Wall -ansi -pedantic	# only for debugging purposes
# OPTIONS = -O3 -Wall -ansi -pedantic
# OPTIONS = -O3 -Wall
# OPTIONS = -O3
# OPTIONS = -DSYSV -Ae			# may be needed for HP-UX

ifdef BUILD_DIST			# distribution build
OPTIONS = -O3 -Wall
endif

CFLAGS = $(OPTIONS) $(SYS_CFLAGS)  $(EXTRA_CFLAGS) $(CONFIG)
LDFLAGS =           $(SYS_LDFLAGS) $(EXTRA_LDFLAGS) -lm


SRCS =	main.c		\
	conf_gfx.c	\
	conf_snd.c	\
	conf_mus.c	\
	conf_hlp.c	\
	init.c		\
	config.c	\
	events.c	\
	tools.c		\
	screens.c	\
	game.c		\
	editor.c	\
	files.c		\
	tape.c		\
	cartoons.c	\
	network.c	\
	netserv.c

OBJS =	main.o		\
	conf_gfx.o	\
	conf_snd.o	\
	conf_mus.o	\
	conf_hlp.o	\
	init.o		\
	config.o	\
	events.o	\
	tools.o		\
	screens.o	\
	game.o		\
	editor.o	\
	files.o		\
	tape.o		\
	cartoons.o	\
	network.o	\
	netserv.o

CNFS =	conf_gfx.h	\
	conf_snd.h	\
	conf_mus.h	\
	conf_chr.c	\
	conf_chr.h	\
	conf_cus.c	\
	conf_cus.h	\
	conf_grp.c	\
	conf_grp.h	\
	conf_e2g.c	\
	conf_esg.c	\
	conf_e2s.c	\
	conf_fnt.c	\
	conf_g2s.c	\
	conf_g2m.c	\
	conf_var.c	\
	conf_act.c

CNFS_CMD = ../Scripts/create_element_defs.pl

TIMESTAMP_FILE = conftime.h

LIBGAME_DIR = libgame
LIBGAME = $(LIBGAME_DIR)/libgame.a

GAME_EM_DIR = game_em
GAME_EM = $(GAME_EM_DIR)/game_em.a

RNDLIBS = $(LIBGAME) $(GAME_EM)

ICONBASE = windows_icon
ifeq ($(PLATFORM),cross-win32)
ifdef SPECIAL_EDITION
ICONSUFFIX = 32x32_$(SPECIAL_EDITION)
else
ICONSUFFIX = 32x32
endif
ICON32X32 = ../graphics/$(ICONBASE)_$(ICONSUFFIX).bmp
ICON = $(ICONBASE).o
endif


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: libgame_dir game_em_dir $(PROGNAME)

$(PROGNAME): $(RNDLIBS) $(TIMESTAMP_FILE) $(OBJS) $(ICON)
	$(CC) $(PROFILING) $(OBJS) $(ICON) $(RNDLIBS) $(LDFLAGS) -o $(PROGNAME)

libgame_dir:
	@$(MAKE) -C $(LIBGAME_DIR)
$(LIBGAME):
	@$(MAKE) -C $(LIBGAME_DIR)

game_em_dir:
	@$(MAKE) -C $(GAME_EM_DIR)
$(GAME_EM):
	@$(MAKE) -C $(GAME_EM_DIR)

auto-conf:
	@for i in $(CNFS); do			\
		echo "$(CNFS_CMD) $$i > $$i";	\
		$(CNFS_CMD) $$i > $$i;		\
	done

conf_gfx.h: conf_gfx.c
	@$(MAKE) auto-conf

conf_snd.h: conf_snd.c
	@$(MAKE) auto-conf

conf_mus.h: conf_mus.c
	@$(MAKE) auto-conf

$(TIMESTAMP_FILE): $(SRCS) $(LIBGAME) $(GAME_EM)
	@date '+"%Y-%m-%d %H:%M"' \
	| sed -e 's/^/#define COMPILE_DATE_STRING /' \
	> $(TIMESTAMP_FILE)

$(ICON):
	$(BMP2ICO) -transparent $(ICONBASE).ico $(ICON32X32)
	echo "$(ICONBASE) ICON $(ICONBASE).ico" | $(WINDRES) -o $(ICON)

.c.o:
	$(CC) $(PROFILING) $(CFLAGS) -c $*.c

clean-obj:
	$(MAKE) -C $(LIBGAME_DIR) clean
	$(MAKE) -C $(GAME_EM_DIR) clean
	$(RM) $(OBJS)
	$(RM) $(LIBGAME)
	$(RM) $(GAME_EM)

clean-ico:
	$(RM) $(ICONBASE).ico
	$(RM) $(ICONBASE).o

clean-bin:
	$(RM) $(PROGNAME)
	$(RM) ../*.exe

clean: clean-obj clean-ico clean-bin


# -----------------------------------------------------------------------------
# run and test targets
# -----------------------------------------------------------------------------

run: all
	$(PROGNAME) --verbose

gdb: all
	gdb -batch -x ../GDB_COMMANDS $(PROGNAME)

valgrind: all
	valgrind -v --leak-check=yes $(PROGNAME) 2> ../valgrind.out


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

dist-clean: clean-obj

tags:
	$(ETAGS) *.[ch] $(LIBGAME_DIR)/*.[ch] $(GAME_EM_DIR)/*.[ch]

depend:
	$(MAKE) -C $(LIBGAME_DIR) depend
	$(MAKE) -C $(GAME_EM_DIR) depend
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
