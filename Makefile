#=============================================================================#
# Makefile for Rocks'n'Diamonds                                               #
# (c) 1995-2002 Holger Schemel, info@artsoft.org                              #
#=============================================================================#

#-----------------------------------------------------------------------------#
# configuration section                                                       #
#-----------------------------------------------------------------------------#

# specify command name of your favorite ANSI C compiler
# (this must be set to "cc" for some systems)
CC = gcc

# specify command name of GNU make on your system
# (this must be set to "gmake" for some systems)
MAKE = make

# specify path to X11 on your system
X11_PATH = /usr/X11R6

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

SRC_DIR = src
MAKE_CMD = $(MAKE) -C $(SRC_DIR)

# DEFAULT_TARGET = x11
DEFAULT_TARGET = sdl

all:
	@$(MAKE_CMD) TARGET=$(DEFAULT_TARGET)

x11:
	@$(MAKE_CMD) TARGET=x11

sdl:
	@$(MAKE_CMD) TARGET=sdl

solaris:
	@$(MAKE_CMD) PLATFORM=solaris TARGET=x11

solaris-sdl:
	@$(MAKE_CMD) PLATFORM=solaris TARGET=sdl

mac:
	@$(MAKE_CMD) PLATFORM=macosx

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

auto-conf:
	./Scripts/create_element_defs.pl --graphics > src/conf_gfx.h
	./Scripts/create_element_defs.pl --sounds   > src/conf_snd.h
	./Scripts/create_element_defs.pl --chars    > src/conf_chr.c
	./Scripts/create_element_defs.pl --custom   > src/conf_cus.c
	./Scripts/create_element_defs.pl --el2gfx   > src/conf_e2g.c

run:
	@$(MAKE_CMD) TARGET=$(DEFAULT_TARGET) && ./rocksndiamonds --verbose

gdb:
	@$(MAKE_CMD) TARGET=$(DEFAULT_TARGET) && gdb ./rocksndiamonds

enginetest:
	./Scripts/make_enginetest.sh

enginetestfast:
	./Scripts/make_enginetest.sh fast

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

dist-macosx:
	./Scripts/make_dist.sh mac . $(MAKE)

upload-unix:
	./Scripts/make_dist.sh unix . upload

upload-msdos:
	./Scripts/make_dist.sh dos . upload

upload-win32:
	./Scripts/make_dist.sh win . upload

upload-macosx:
	./Scripts/make_dist.sh mac . upload

dist-clean:
	@$(MAKE_CMD) dist-clean

dist-build-all:
	$(MAKE) clean
	@BUILD_DIST=TRUE $(MAKE) x11		; $(MAKE) dist-clean
	@BUILD_DIST=TRUE $(MAKE) cross-win32	; $(MAKE) dist-clean
	@BUILD_DIST=TRUE $(MAKE) cross-msdos	; $(MAKE) dist-clean

dist-all: dist-build-all dist-unix dist-msdos dist-win32 dist-macosx

upload-all: upload-unix upload-msdos upload-win32 upload-macosx

depend dep:
	$(MAKE_CMD) depend
