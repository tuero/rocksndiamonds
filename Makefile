#=============================================================================#
# Makefile for Rocks'n'Diamonds 1.2                                           #
# (c) 1995-98 Holger Schemel, aeglos@valinor.owl.de                           #
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

# specify path to X11 on your system
# if undefined, use system defaults (works fine with Linux/gcc)
# X11_PATH = /usr/X11

# specify path to install game data (graphics, sounds, levels, scores)
# default is '.', so you can play without installing game data somewhere
# GAME_DIR = /usr/local/games

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

#-----------------------------------------------------------------------------#
# you should not need to change anything below                                #
#-----------------------------------------------------------------------------#

.EXPORT_ALL_VARIABLES:

MAKE = make

SRC_DIR = src
MAKE_CMD = @$(MAKE) -C $(SRC_DIR)


all:
	$(MAKE_CMD)

solaris:
	$(MAKE_CMD) PLATFORM=solaris

clean:
	$(MAKE_CMD) clean
