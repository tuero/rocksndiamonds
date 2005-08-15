# =============================================================================
# Rocks'n'Diamonds Makefile (src)
# -----------------------------------------------------------------------------
# (c) 1995-2005 Holger Schemel <info@artsoft.org>
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

.EXPORT_ALL_VARIABLES:

ifndef PLATFORM			# no platform specified -- try auto detection
ifdef COMSPEC
PLATFORM = msdos
else
PLATFORM = unix
endif
endif

ifdef X11_PATH			# path to X11 specified by top level Makefile
XINC_PATH = $(X11_PATH)/include
XLIB_PATH = $(X11_PATH)/lib
X11_INCL = -I$(XINC_PATH)
X11_LIBS = -L$(XLIB_PATH) -lX11
endif

AR = ar
RANLIB = ranlib
ETAGS = etags

BMP2ICO = bmp2ico
WINDRES = windres

ifeq ($(PLATFORM),msdos)	# MS-DOS native compiling

RM = del

PROGBASE = rocks
PROGNAME = ../$(PROGBASE).exe

SYS_CFLAGS = -DTARGET_X11
SYS_LDFLAGS = -s -lalleg

else				# Unix or cross-compiling for MS-DOS and Win32

RM = rm -f

PROGBASE = rocksndiamonds
PROGNAME = ../$(PROGBASE)

ifeq ($(PLATFORM),solaris)
EXTRA_LDFLAGS = -lnsl -lsocket -R$(XLIB_PATH)
PLATFORM = unix
endif

ifeq ($(PLATFORM),macosx)
PLATFORM = unix
TARGET = sdl
endif

ifeq ($(PLATFORM),os2)
RANLIB = echo
PROGNAME = ../$(PROGBASE).exe
EXTRA_CFLAGS = -Zbin-files -D__ST_MT_ERRNO__ -Zmtd -fomit-frame-pointer
EXTRA_LDFLAGS = -lsocket
PLATFORM = unix
TARGET = x11
endif

ifeq ($(PLATFORM),unix)
PROFILING_FLAGS = -pg
endif

ifeq ($(PLATFORM),cross-msdos)
PROGBASE = rocks
PROGNAME = ../$(PROGBASE).exe
TARGET = allegro
endif

ifeq ($(PLATFORM),cross-win32)
EXTRA_LDFLAGS = -lshfolder -lwsock32
PROGNAME = ../$(PROGBASE).exe
TARGET = sdl
endif

ifeq ($(TARGET),x11)
SYS_CFLAGS  = -DTARGET_X11 $(X11_INCL)
SYS_LDFLAGS = $(X11_LIBS)
endif

ifeq ($(TARGET),sdl)
SYS_CFLAGS  = -DTARGET_SDL $(shell sdl-config --cflags)
SDL_LIBS = -lSDL_image -lSDL_mixer -lSDL_net -lsmpeg
SYS_LDFLAGS = $(SDL_LIBS) $(shell sdl-config --libs)
endif

ifeq ($(TARGET),sdl-static)
SYS_CFLAGS  = -DTARGET_SDL $(shell sdl-config --cflags)
SDL_LIBS = -lSDL_image -lSDL_mixer -lSDL_net -lsmpeg
SYS_LDFLAGS = $(SDL_LIBS) $(shell sdl-config --static-libs)
endif

ifeq ($(TARGET),allegro)
SYS_CFLAGS = -DTARGET_ALLEGRO -I$(CROSS_PATH_MSDOS)/include
SYS_LDFLAGS = -lalleg
endif

endif				# (PLATFORM != msdos)


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


CONFIG = $(CONFIG_GAME_DIR) $(CONFIG_SCORE_ENTRIES) $(JOYSTICK)
# DEBUG = -DDEBUG -g
# PROFILING = $(PROFILING_FLAGS)

# OPTIONS = $(DEBUG) -Wall			# only for debugging purposes
# OPTIONS = $(DEBUG) -O3 -Wall			# only for debugging purposes
OPTIONS = $(DEBUG) -Wall			# only for debugging purposes
# OPTIONS = $(DEBUG) -Wall -ansi -pedantic	# only for debugging purposes
# OPTIONS = -O3 -Wall -ansi -pedantic
# OPTIONS = -O3 -Wall
# OPTIONS = -O3
# OPTIONS = -DSYSV -Ae				# may be needed for HP-UX

ifdef BUILD_DIST				# distribution build
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
	conf_g2m.c

CNFS_CMD = ../Scripts/create_element_defs.pl

TIMESTAMP_FILE = conftime.h

LIBGAME_DIR = libgame
LIBGAME = $(LIBGAME_DIR)/libgame.a

GAME_EM_DIR = game_em
GAME_EM = $(GAME_EM_DIR)/game_em.a

RNDLIBS = $(LIBGAME) $(GAME_EM)

ICONBASE = windows_icon
ifeq ($(PLATFORM),cross-win32)
ICON32X32 = ../graphics/$(ICONBASE)_32x32.bmp
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
	@date '+"[%Y-%m-%d %H:%M]"' \
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
