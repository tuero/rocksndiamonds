#=============================================================================#
# Makefile for Rocks'n'Diamonds                                               #
# (c) 1995-2000 Holger Schemel, info@artsoft.org                              #
#=============================================================================#

#-----------------------------------------------------------------------------#
# configuration section                                                       #
#-----------------------------------------------------------------------------#

# specify your favorite ANSI C compiler
CC = gcc

# specify path to X11 on your system
# if undefined, use system defaults (works with Linux/gcc/libc5)
X11_PATH = /usr/X11

# specify directory for read-only game data (like graphics, sounds, levels)
# default is '.', so you can play without installing game data somewhere
# RO_GAME_DIR = /usr/games

# specify directory for writable game data (like highscore files)
# default is '.', so you can play without installing game data somewhere
# RW_GAME_DIR = /var/games

# uncomment this if your system has no joystick include file
# JOYSTICK = -DNO_JOYSTICK

# choose if you want to allow many global score file entries for one player
# default is 'MANY_PER_NAME'
# when installing the game in a multi user environment, choose this
# SCORE_ENTRIES = ONE_PER_NAME
# when installing the game in a single user environment, choose this
# SCORE_ENTRIES = MANY_PER_NAME

# specify paths for cross-compiling (only needed for MS-DOS and Win32 build)
CROSS_PATH_MSDOS=/usr/local/cross-msdos/i386-msdosdjgpp
CROSS_PATH_WIN32=/usr/local/cross-tools/i386-mingw32msvc

#-----------------------------------------------------------------------------#
# you should not need to change anything below                                #
#-----------------------------------------------------------------------------#

.EXPORT_ALL_VARIABLES:

MAKE = make

SRC_DIR = src
MAKE_CMD = $(MAKE) -C $(SRC_DIR)

all:
	@$(MAKE_CMD) TARGET=x11

x11:
	@$(MAKE_CMD) TARGET=x11

sdl:
	@$(MAKE_CMD) TARGET=sdl

solaris:
	@$(MAKE_CMD) PLATFORM=solaris TARGET=x11

solaris-sdl:
	@$(MAKE_CMD) PLATFORM=solaris TARGET=sdl

msdos:
	@$(MAKE_CMD) PLATFORM=msdos

cross-msdos:
	@PATH=$(CROSS_PATH_MSDOS)/bin:${PATH} $(MAKE_CMD) PLATFORM=cross-msdos

cross-win32:
	@PATH=$(CROSS_PATH_WIN32)/bin:${PATH} $(MAKE_CMD) PLATFORM=cross-win32

clean:
	@$(MAKE_CMD) clean


#-----------------------------------------------------------------------------#
# development only stuff                                                      #
#-----------------------------------------------------------------------------#

backup:
	./Scripts/make_backup.sh src

backup_lev:
	./Scripts/make_backup.sh lev

backup_gfx:
	./Scripts/make_backup.sh gfx

dist-unix:
	./Scripts/make_dist.sh unix .

dist-msdos:
	./Scripts/make_dist.sh dos .

dist-win32:
	./Scripts/make_dist.sh win .

dist-clean:
	@$(MAKE_CMD) dist-clean

dist-build-all:
	$(MAKE) clean
	@BUILD_DIST=TRUE $(MAKE) x11		; $(MAKE) dist-clean
	@BUILD_DIST=TRUE $(MAKE) cross-win32	; $(MAKE) dist-clean
	@BUILD_DIST=TRUE $(MAKE) cross-msdos	; $(MAKE) dist-clean

dist-all: dist-build-all dist-unix dist-msdos dist-win32

depend dep:
	$(MAKE_CMD) depend
