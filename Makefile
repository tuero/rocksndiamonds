#=============================================================================#
# Makefile for Rocks'n'Diamonds 1.4.0                                         #
# (c) 1995-1999 Holger Schemel, aeglos@valinor.owl.de                         #
#=============================================================================#

#-----------------------------------------------------------------------------#
# configuration section                                                       #
#-----------------------------------------------------------------------------#

# specify your favorite ANSI C compiler
CC = gcc

# explicitely choose your platform, if defaults doesn't work right
# needed for SUN/Solaris; Linux and DOS work fine with auto detection
# PLATFORM = solaris
# PLATFORM = unix
# PLATFORM = dos
# PLATFORM = windows

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

# uncomment this if your system has no sound
# SOUNDS = -DNO_SOUNDS

# choose if you want to allow many global score file entries for one player
# default is 'MANY_PER_NAME'
# when installing the game in a multi user environment, choose this
# SCORE_ENTRIES = ONE_PER_NAME
# when installing the game in a single user environment, choose this
# SCORE_ENTRIES = MANY_PER_NAME

# specify path for cross-compiling (only needed for Windows build)
CROSS_PATH=/usr/local/cross-tools/i386-mingw32/bin

#-----------------------------------------------------------------------------#
# you should not need to change anything below                                #
#-----------------------------------------------------------------------------#

.EXPORT_ALL_VARIABLES:

MAKE = make

SRC_DIR = src
MAKE_CMD = $(MAKE) -C $(SRC_DIR)

all:
	@$(MAKE_CMD) TARGET=sdl

x11:
	@$(MAKE_CMD) TARGET=x11

sdl:
	@$(MAKE_CMD) TARGET=sdl

sdl_old:
	@$(MAKE_CMD) TARGET=sdl_old_10

solaris:
	@$(MAKE_CMD) PLATFORM=solaris

windows:
	@PATH=$(CROSS_PATH):${PATH} $(MAKE_CMD) PLATFORM=windows

clean:
	@$(MAKE_CMD) clean
