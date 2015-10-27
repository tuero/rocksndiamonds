# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2015 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  http://www.artsoft.org/
# -----------------------------------------------------------------------------
# Makefile
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

# command name of your favorite ANSI C compiler
# (this must be set to "cc" for some systems)
CC = gcc

# command name of GNU make on your system
# (this must be set to "gmake" for some systems)
MAKE = make

# directory for read-only game data (like graphics, sounds, levels)
# default is '.' to be able to run program without installation
# RO_GAME_DIR = /usr/games
# use the following setting for Debian / Ubuntu installations:
# RO_GAME_DIR = /usr/share/games/rocksndiamonds

# directory for writable game data (like highscore files)
# default is '.' to be able to run program without installation
# RW_GAME_DIR = /var/games
# use the following setting for Debian / Ubuntu installations:
# RW_GAME_DIR = /var/games/rocksndiamonds

# uncomment if system has no joystick include file
# JOYSTICK = -DNO_JOYSTICK

# choose if more than one global score file entry for one player is allowed
# (default: MANY_PER_NAME)
# uncomment to install game in multi-user environment
# SCORE_ENTRIES = ONE_PER_NAME
# uncomment to install game in single-user environment (default)
# SCORE_ENTRIES = MANY_PER_NAME

# path for cross-compiling (only needed for non-native Windows build)
CROSS_PATH_WIN32 = /usr/local/cross-tools/i386-mingw32msvc


# -----------------------------------------------------------------------------
# there should be no need to change anything below
# -----------------------------------------------------------------------------

.EXPORT_ALL_VARIABLES:

SRC_DIR = src
MAKE_CMD = $(MAKE) -C $(SRC_DIR)


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all:
	@$(MAKE_CMD)

sdl:
	@$(MAKE_CMD) TARGET=sdl

sdl2:
	@$(MAKE_CMD) TARGET=sdl2

mac:
	@$(MAKE_CMD) PLATFORM=macosx

cross-win32:
	@PATH=$(CROSS_PATH_WIN32)/bin:${PATH} $(MAKE_CMD) PLATFORM=cross-win32

strip:
	@$(MAKE_CMD) strip

clean:
	@$(MAKE_CMD) clean

clean-git:
	@$(MAKE_CMD) clean-git


# -----------------------------------------------------------------------------
# development, test, distribution build and packaging targets
# -----------------------------------------------------------------------------

auto-conf:
	@$(MAKE_CMD) auto-conf

run: all
	@$(MAKE_CMD) run

gdb: all
	@$(MAKE_CMD) gdb

valgrind: all
	@$(MAKE_CMD) valgrind

enginetest: all
	./Scripts/make_enginetest.sh

enginetestcustom: all
	./Scripts/make_enginetest.sh custom

enginetestfast: all
	./Scripts/make_enginetest.sh fast

enginetestnew: all
	./Scripts/make_enginetest.sh new

leveltest: all
	./Scripts/make_enginetest.sh leveltest

levelsketch_images: all
	./Scripts/make_levelsketch_images.sh

dist-clean:
	@$(MAKE_CMD) dist-clean

dist-build-unix:
	@BUILD_DIST=TRUE $(MAKE)

dist-build-win32:
	@BUILD_DIST=TRUE $(MAKE) cross-win32

dist-build-macosx:
	@BUILD_DIST=TRUE $(MAKE)

dist-package-unix:
	./Scripts/make_dist.sh package unix

dist-package-win32:
	./Scripts/make_dist.sh package win

dist-package-macosx:
	./Scripts/make_dist.sh package mac

dist-upload-unix:
	./Scripts/make_dist.sh upload unix

dist-upload-win32:
	./Scripts/make_dist.sh upload win

dist-upload-macosx:
	./Scripts/make_dist.sh upload mac

dist-package-all:
	$(MAKE) dist-package-unix
	$(MAKE) dist-package-win32
	$(MAKE) dist-package-macosx

dist-upload-all:
	$(MAKE) dist-upload-unix
	$(MAKE) dist-upload-win32
	$(MAKE) dist-upload-macosx

package-all: dist-package-all

upload-all: dist-upload-all

tags:
	$(MAKE_CMD) tags

depend dep:
	$(MAKE_CMD) depend
