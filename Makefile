# =============================================================================
# Rocks'n'Diamonds Makefile
# -----------------------------------------------------------------------------
# (c) 1995-2006 Holger Schemel <info@artsoft.org>
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
CROSS_PATH_MSDOS=/usr/local/cross-msdos/i386-msdosdjgpp
CROSS_PATH_WIN32=/usr/local/cross-tools/i386-mingw32msvc

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

cross-win32-jue:
	@PATH=$(CROSS_PATH_WIN32)/bin:${PATH} $(MAKE_CMD) PLATFORM=cross-win32 \
							  SPECIAL_ICON=jue

clean:
	@$(MAKE_CMD) clean


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

auto-conf:
	@$(MAKE_CMD) auto-conf

run: all
	@./rocksndiamonds --verbose

gdb: all
	@gdb -batch -x GDB_COMMANDS ./rocksndiamonds

valgrind: all
	@valgrind -v --leak-check=yes ./rocksndiamonds 2> valgrind.out

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

dist-build-unix:
	@BUILD_DIST=TRUE $(MAKE) x11

dist-build-win32:
	@BUILD_DIST=TRUE $(MAKE) cross-win32

dist-build-msdos:
	@BUILD_DIST=TRUE $(MAKE) cross-msdos

dist-build-all:
	$(MAKE) clean
	$(MAKE) dist-build-unix		; $(MAKE) dist-clean
	$(MAKE) dist-build-win32	; $(MAKE) dist-clean
#	$(MAKE) dist-build-msdos	; $(MAKE) dist-clean

# dist-all: dist-build-all dist-unix dist-msdos dist-win32 dist-macosx
dist-all: dist-build-all dist-unix dist-win32 dist-macosx

# upload-all: upload-unix upload-msdos upload-win32 upload-macosx
upload-all: upload-unix upload-win32 upload-macosx

tags:
	$(MAKE_CMD) tags

depend dep:
	$(MAKE_CMD) depend
