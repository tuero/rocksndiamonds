# =============================================================================
# Rocks'n'Diamonds Makefile
# -----------------------------------------------------------------------------
# (c) 1995-2007 Holger Schemel <info@artsoft.org>
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

# path to X11 on your system
X11_PATH = /usr/X11R6

# directory for read-only game data (like graphics, sounds, levels)
# default is '.' to be able to run program without installation
# RO_GAME_DIR = /usr/games

# directory for writable game data (like highscore files)
# default is '.' to be able to run program without installation
# RW_GAME_DIR = /var/games

# uncomment if system has no joystick include file
# JOYSTICK = -DNO_JOYSTICK

# choose if more than one global score file entry for one player is allowed
# (default: MANY_PER_NAME)
# uncomment to install game in multi-user environment
# SCORE_ENTRIES = ONE_PER_NAME
# uncomment to install game in single-user environment (default)
# SCORE_ENTRIES = MANY_PER_NAME

# paths for cross-compiling (only needed for non-native MS-DOS and Win32 build)
CROSS_PATH_MSDOS = /usr/local/cross-msdos/i386-msdosdjgpp
CROSS_PATH_WIN32 = /usr/local/cross-tools/i386-mingw32msvc

# compile special edition of R'n'D instead of the normal (classic) version
# SPECIAL_EDITION = rnd_jue


# -----------------------------------------------------------------------------
# there should be no need to change anything below
# -----------------------------------------------------------------------------

.EXPORT_ALL_VARIABLES:

SRC_DIR = src
MAKE_CMD = $(MAKE) -C $(SRC_DIR)

# DEFAULT_TARGET = x11
DEFAULT_TARGET = sdl


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

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

mac-static:
	@$(MAKE_CMD) PLATFORM=macosx TARGET=sdl-static

msdos:
	@$(MAKE_CMD) PLATFORM=msdos

os2:
	@$(MAKE_CMD) PLATFORM=os2

cross-msdos:
	@PATH=$(CROSS_PATH_MSDOS)/bin:${PATH} $(MAKE_CMD) PLATFORM=cross-msdos

cross-win32:
	@PATH=$(CROSS_PATH_WIN32)/bin:${PATH} $(MAKE_CMD) PLATFORM=cross-win32

clean:
	@$(MAKE_CMD) clean


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

backup:
	./Scripts/make_backup.sh src 1

backup2:
	./Scripts/make_backup.sh src 2

backup3:
	./Scripts/make_backup.sh src 3

backup_lev:
	./Scripts/make_backup.sh lev

backup_gfx:
	./Scripts/make_backup.sh gfx

# prerelease:
#	./Scripts/make_prerelease.sh

dist-clean:
	@$(MAKE_CMD) dist-clean

dist-build-unix:
	@BUILD_DIST=TRUE $(MAKE) x11

dist-build-msdos:
	@BUILD_DIST=TRUE $(MAKE) cross-msdos

dist-build-win32:
	@BUILD_DIST=TRUE $(MAKE) cross-win32

dist-build-macosx:
	# (this is done by "dist-package-macosx" target)

dist-package-unix:
	./Scripts/make_dist.sh unix .

dist-package-msdos:
	./Scripts/make_dist.sh dos .

dist-package-win32:
	./Scripts/make_dist.sh win .

dist-package-macosx:
	./Scripts/make_dist.sh mac . $(MAKE)

dist-upload-unix:
	./Scripts/make_dist.sh unix . upload

dist-upload-msdos:
	./Scripts/make_dist.sh dos . upload

dist-upload-win32:
	./Scripts/make_dist.sh win . upload

dist-upload-macosx:
	./Scripts/make_dist.sh mac . upload

dist-build-all:
	$(MAKE) clean
	$(MAKE) dist-build-unix		; $(MAKE) dist-clean
	$(MAKE) dist-build-win32	; $(MAKE) dist-clean
#	$(MAKE) dist-build-msdos	; $(MAKE) dist-clean

dist-package-all:
	$(MAKE) dist-package-unix
	$(MAKE) dist-package-win32
	$(MAKE) dist-package-macosx
#	$(MAKE) dist-package-msdos

dist-upload-all:
	$(MAKE) dist-upload-unix
	$(MAKE) dist-upload-win32
	$(MAKE) dist-upload-macosx
#	$(MAKE) dist-upload-msdos

dist-all: dist-build-all dist-package-all

upload-all: dist-upload-all

tags:
	$(MAKE_CMD) tags

depend dep:
	$(MAKE_CMD) depend
