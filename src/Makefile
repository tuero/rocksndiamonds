# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2015 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  http://www.artsoft.org/
#
# (2019) Jake Tuero (tuero@ualberta.ca)
# - Modifications made to include ai library for custom solver controllers.
# -----------------------------------------------------------------------------
# src/Makefile
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

.EXPORT_ALL_VARIABLES:

ifndef PLATFORM				# unknown platform -- default to Unix
PLATFORM = unix
endif

AR = ar
RANLIB = ranlib
ETAGS = etags
RM = rm -f

CONVERT = convert
WINDRES = windres

CONVERT_ICON_ARGS = -transparent black -background transparent

PROGBASE = rocksndiamonds
PROGNAME = ../$(PROGBASE)

EDITION ?= default


# -----------------------------------------------------------------------------
# configuring platform
# -----------------------------------------------------------------------------

ifeq ($(PLATFORM),macosx)		# compiling on Mac OS X
EXTRA_LDFLAGS = -lstdc++
override PLATFORM = unix
endif

ifeq ($(PLATFORM),unix)			# compiling on Unix/Linux (generic)
PROFILING_FLAGS = -pg
endif

ifeq ($(PLATFORM),cross-win32)		# cross-compiling to Windows
PROGNAME = ../$(PROGBASE).exe
EXTRA_LDFLAGS = -lshfolder -lwsock32
endif


# -----------------------------------------------------------------------------
# configuring target
# -----------------------------------------------------------------------------

ifndef TARGET				# auto-detect compiling for SDL2
	SDL_VERSION := $(shell sdl2-config --version 2> /dev/null)
	ifdef SDL_VERSION
		TARGET = sdl2
	else
		$(error SDL2 library not found)
	endif
endif

# $(info Using SDL version $(SDL_VERSION) [TARGET == $(TARGET)])

ifeq ($(TARGET),sdl)			# compiling for SDL target
	$(error SDL library version 1.2 not supported anymore)
endif

ifeq ($(TARGET),sdl2)			# compiling for SDL2 target
SYS_CFLAGS = -DTARGET_SDL2 $(shell sdl2-config --cflags)
SDL_LIBS = -lSDL2_image -lSDL2_mixer -lSDL2_net
SYS_LDFLAGS = $(SDL_LIBS) $(shell sdl2-config --libs)
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

CONFIG = $(CONFIG_RO_GAME_DIR) $(CONFIG_RW_GAME_DIR) $(JOYSTICK)

DEBUG = -DDEBUG -g

# PROFILING = $(PROFILING_FLAGS)

# OPTIONS = $(DEBUG) -Wall			# only for debugging purposes
# OPTIONS = $(DEBUG) -O3 -Wall			# only for debugging purposes
# OPTIONS = $(DEBUG) -Wall			# only for debugging purposes
OPTIONS = -g -Wall -Wstrict-prototypes -Wmissing-prototypes
# OPTIONS = $(DEBUG) -Wall -ansi -pedantic	# only for debugging purposes
# OPTIONS = -O3 -Wall -ansi -pedantic
# OPTIONS = -O3 -Wall
# OPTIONS = -O3

ifdef BUILD_DIST			# distribution build
SYS_LDFLAGS := $(shell echo $(SYS_LDFLAGS) |	\
		       sed -e "s%-rpath,[^ ]*%-rpath,'\$$ORIGIN/lib'%")
OPTIONS = -O3 -Wall
endif

CFLAGS = $(OPTIONS) -MMD -MP $(SYS_CFLAGS) $(EXTRA_CFLAGS) $(CONFIG)
CFLAGS_HEADLESS = $(OPTIONS) -MMD -MP -DHEADLESS $(EXTRA_CFLAGS) $(CONFIG)
CXXFLAGS := -Wall -Wextra -g -O3 -std=c++14 -MMD -MP $(SYS_CFLAGS) $(EXTRA_CFLAGS) $(CONFIG)
CXXFLAGS += -I${CURDIR}/ai/includes
# CXXFLAGS += -I${CURDIR}/ai/includes -I/usr/local/include -L/usr/local/lib -lSDL2
CXXFLAGS_HEADLESS := -Wall -Wextra -g -O3 -std=c++14 -MMD -DHEADLESS -I${CURDIR}/ai/includes $(EXTRA_CFLAGS) $(CONFIG)
LDFLAGS =           $(SYS_LDFLAGS) $(EXTRA_LDFLAGS) -lm -lz
LDFLAGS_HEADLESS = -lm -lz

BIN_DIR = ../bin
BUILD_DIR = ../build
BUILD_CUSTOM_DIR = $(BUILD_DIR)/custom
BUILD_HEADLESS_DIR = $(BUILD_DIR)/headless
PROGNAMES = ../rocksndiamonds ../rocksndiamonds_headless


SRCS_ROOT =	main.c conf_gfx.c conf_snd.c conf_mus.c	conf_hlp.c	init.c config.c events.c 			\
		tools.c screens.c game.c editor.c files.c tape.c anim.c network.c netserv.c
		
SRCS_LIBGAME = libgame/system.c libgame/gadgets.c libgame/text.c libgame/sound.c 					\
		libgame/snapshot.c libgame/joystick.c libgame/image.c libgame/random.c						\
		libgame/hash.c libgame/setup.c libgame/misc.c libgame/sdl.c	 libgame/zip/ioapi.c			\
		libgame/zip/iowin32.c libgame/zip/unzip.c libgame/zip/miniunz.c
		
SRCS_GAME_EM = game_em/cave.c game_em/convert.c game_em/graphics.c	 game_em/init.c	 				\
		game_em/input.c game_em/main.c game_em/sound.c game_em/synchro_1.c game_em/synchro_2.c		\
		game_em/synchro_3.c	game_em/tab_generate.c game_em/ulaw_generate.c
	
SRCS_GAME_MM = game_mm/mm_init.c game_mm/mm_main.c game_mm/mm_game.c game_mm/mm_files.c				\
		game_mm/mm_tools.c

SRCS_GAME_SP = game_sp/init.c game_sp/file.c game_sp/main.c game_sp/vb_lib.c game_sp/ASM.c			\
		game_sp/BugsTerminals.c game_sp/DDScrollBuffer.c game_sp/DDSpriteBuffer.c 					\
		game_sp/Display.c game_sp/DoGameStuff.c game_sp/Electrons.c game_sp/Explosions.c		 	\
		game_sp/Globals.c game_sp/Infotrons.c game_sp/InitGameConditions.c game_sp/Input.c			\
		game_sp/MainForm.c game_sp/MainGameLoop.c game_sp/Murphy.c game_sp/OrangeDisk.c				\
		game_sp/SnikSnaks.c	 game_sp/Sound.c game_sp/Zonk.c

SRCS_AI = ai/ai_entry.cpp 									\
					ai/hash_table.cpp 									\
					ai/controller/controller.cpp 					\
					ai/controller/default/default.cpp 					\
					ai/controller/mcts/mcts.cpp 						\
					ai/controller/mcts/tree_node.cpp	  				\
					ai/controller/options/option_factory.cpp 			\
					ai/controller/options/base_option.cpp 				\
					ai/controller/options/option_single_step.cpp 		\
					ai/controller/options/option_to_sprite.cpp 			\
					ai/controller/options/option_collectible_sprite.cpp 			\
					ai/controller/options/option_to_exit.cpp 			\
					ai/controller/options/option_to_rock.cpp 			\
					ai/controller/options/option_wait_rock.cpp			\
					ai/controller/options/option_push_rock.cpp 			\
					ai/controller/replay/replay.cpp 	 				\
					ai/controller/two_level_search/two_level_search.cpp \
					ai/engine/action.cpp 						\
					ai/engine/engine_helper.cpp 				\
					ai/engine/game_state.cpp					\
					ai/engine/game_state_abstract.cpp					\
					ai/level_programming/level_programming.cpp				\
					ai/util/timer.cpp 						\
					ai/util/rng.cpp 							\
					ai/util/logger.cpp 				\
					ai/util/statistics.cpp					\
					ai/util/summary_window.cpp 				\
					ai/tests/test_engine_speed.cpp 			\
					ai/tests/test_rng.cpp 

DIR_SRCS = libgame libgame/zip game_em game_mm game_sp ai ai/controller ai/controller/default \
		ai/controller/mcts ai/controller/options ai/controller/two_level_search ai/controller/replay \
		ai/engine ai/level_programming ai/util ai/tests
DIR_LOGS = ai/replays ai/logs ai/stats

SRCS_C 		= $(SRCS_ROOT) $(SRCS_LIBGAME) $(SRCS_GAME_EM) $(SRCS_GAME_MM) $(SRCS_GAME_SP) 
SRCS_CPP	= $(SRCS_AI)
OBJS = 	$(SRCS_C:.c=.o) $(SRCS_CPP:.cpp=.o)
# DEPENDENCIES := $(OBJS:.o=.d)
HEADERS := $(OBJS:.o=.h)
RNDLIBS_CUSTOM 	= 	$(addprefix $(BUILD_CUSTOM_DIR)/, $(OBJS))	
RNDLIBS_HEADLESS = 	$(addprefix $(BUILD_HEADLESS_DIR)/, $(OBJS))
DEPENDENCIES := $(RNDLIBS_CUSTOM:.o=.d) $(RNDLIBS_HEADLESS:.o=.d)

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

CNFS_CMD = ../build-scripts/create_element_defs.pl

TIMESTAMP_FILE = conftime.h
TIMESTAMP_FORMAT = %Y-%m-%d %H:%M

# use SOURCE_DATE_EPOCH, or else last Git commit date, or else current date
SOURCE_DATE_EPOCH ?= $(shell test -d ../.git && test `git ls-files -m | wc -l` -eq 0 && git show -s --format=%ct || date +%s)

# get source date string from either GNU / Linux or BSD / Mac OS X style "date"
SOURCE_DATE_STRING := $(shell date -u -d "@$(SOURCE_DATE_EPOCH)" "+$(TIMESTAMP_FORMAT)"  2>/dev/null || date -u -r "$(SOURCE_DATE_EPOCH)" "+$(TIMESTAMP_FORMAT)" 2>/dev/null || date -u "+$(TIMESTAMP_FORMAT)")


AUTOCONF = conf_gfx.h conf_snd.h conf_mus.h

ICONBASE = windows_icon
ICON_BASEPATH = ../Special/Icons/windows_icons

ifeq ($(PLATFORM),cross-win32)
ICON_PATH = $(ICON_BASEPATH)/$(EDITION)
ICON = $(ICONBASE).o
endif

GRAPHICS_DIR = ../graphics


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: $(AUTOCONF) dir graphics_dir custom

dir:
	@mkdir -p $(addprefix $(BUILD_CUSTOM_DIR)/, $(DIR_SRCS))
	@mkdir -p $(addprefix $(BUILD_HEADLESS_DIR)/, $(DIR_SRCS))
	@mkdir -p $(DIR_LOGS)

custom: rocksndiamonds rocksndiamonds_headless

rocksndiamonds: $(TIMESTAMP_FILE) $(RNDLIBS_CUSTOM)
	@$(CXX) $(PROFILING) $(RNDLIBS_CUSTOM) $(LDFLAGS) -o ../$@

rocksndiamonds_headless: $(TIMESTAMP_FILE) $(RNDLIBS_HEADLESS)
	@$(CXX) $(PROFILING) $(RNDLIBS_HEADLESS) $(LDFLAGS_HEADLESS) -o ../$@


$(BUILD_CUSTOM_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_HEADLESS_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS_HEADLESS) -c $< -o $@

$(BUILD_CUSTOM_DIR)/%.o: %.c
	$(CC) $(PROFILING) $(CFLAGS) -c $< -o $@
$(BUILD_HEADLESS_DIR)/%.o: %.c
	$(CC) $(PROFILING) $(CFLAGS_HEADLESS) -c $< -o $@


auto-conf:
	@for i in $(CNFS); do			\
		echo "$(CNFS_CMD) $$i > $$i";	\
		$(CNFS_CMD) $$i > $$i;		\
	done

auto-conf-clean:
	@for i in $(CNFS); do			\
		echo "$(RM) $$i";		\
		$(RM) $$i;			\
	done

conf_gfx.h: conf_gfx.c $(CNFS_CMD)
	@$(MAKE) auto-conf

conf_snd.h: conf_snd.c $(CNFS_CMD)
	@$(MAKE) auto-conf

conf_mus.h: conf_mus.c $(CNFS_CMD)
	@$(MAKE) auto-conf

$(TIMESTAMP_FILE): $(SRCS)
	@echo '#define SOURCE_DATE_STRING "$(SOURCE_DATE_STRING)"' \
	> $(TIMESTAMP_FILE)

$(ICON):
	$(CONVERT) $(ICON_PATH)/*.png $(CONVERT_ICON_ARGS) $(ICONBASE).ico
	echo "$(ICONBASE) ICON $(ICONBASE).ico" | $(WINDRES) -o $(ICON)
	#	$(CONVERT) $(ICON32X32) $(CONVERT_ICON_ARGS) $(ICONBASE).ico

graphics_dir:
	@test -f $(GRAPHICS_DIR)/Makefile && $(MAKE) -C $(GRAPHICS_DIR) || true

clean-obj:
	$(RM) -r $(BUILD_CUSTOM_DIR)/*
	$(RM) -r $(BUILD_HEADLESS_DIR)/*

clean-ico:
	$(RM) $(ICONBASE).ico
	$(RM) $(ICONBASE).o

clean-bin:
	$(RM) $(PROGNAME)
	$(RM) $(PROGNAMES)
	$(RM) ../*.exe

clean: clean-obj clean-ico clean-bin

clean-git: clean auto-conf-clean
	@$(MAKE) -C $(GRAPHICS_DIR) clean

dist-clean: clean-obj clean-ico


# -----------------------------------------------------------------------------
# run and test targets
# -----------------------------------------------------------------------------

run:
	cd .. && ./$(PROGBASE) --verbose

gdb:
	cd .. && gdb -batch -x GDB_COMMANDS ./$(PROGBASE)

valgrind:
	cd .. && valgrind -v --leak-check=yes ./$(PROGBASE) 2> valgrind.out


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

tags:
	$(ETAGS) *.[ch] $(LIBGAME_DIR)/*.[ch] $(GAME_EM_DIR)/*.[ch] $(GAME_SP_DIR)/*.[ch] $(GAME_MM_DIR)/*.[ch]


-include $(DEPENDENCIES)

