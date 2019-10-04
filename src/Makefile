# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2015 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  http://www.artsoft.org/
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
OPTIONS = $(DEBUG) -Wall -Wstrict-prototypes -Wmissing-prototypes
# OPTIONS = $(DEBUG) -Wall -ansi -pedantic	# only for debugging purposes
# OPTIONS = -O3 -Wall -ansi -pedantic
# OPTIONS = -O3 -Wall
# OPTIONS = -O3

ifdef BUILD_DIST			# distribution build
SYS_LDFLAGS := $(shell echo $(SYS_LDFLAGS) |	\
		       sed -e "s%-rpath,[^ ]*%-rpath,'\$$ORIGIN/lib'%")
OPTIONS = -O3 -Wall
endif


CFLAGS = $(OPTIONS) -MMD $(SYS_CFLAGS)  $(EXTRA_CFLAGS) $(CONFIG)
# CPPFLAGS := -Wall -Wextra -g -O3 -std=c++14 $(SYS_CFLAGS) $(EXTRA_CFLAGS) $(CONFIG)
# CPPFLAGS += -I${CURDIR}/ai/includes
CXXFLAGS := -Wall -Wextra -g -O3 -std=c++14 -MMD $(SYS_CFLAGS) $(EXTRA_CFLAGS) $(CONFIG)
CXXFLAGS += -I${CURDIR}/ai/includes
LDFLAGS =           $(SYS_LDFLAGS) $(EXTRA_LDFLAGS) -lm -lz

BIN_DIR = ../bin
BUILD_DIR = ../build
BUILD_CUSTOM_DIR = $(BUILD_DIR)/custom
BUILD_HEADLESS_DIR = $(BUILD_DIR)/headless
PROGNAMES = ../rocksndiamonds ../rocksndiamonds_headless

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
	anim.c		\
	network.c	\
	netserv.c   

OBJS 		= 	$(SRCS:.c=.o)

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


LIBGAME_DIR = 		libgame
LIBGAME_CUSTOM_DIR = $(BUILD_CUSTOM_DIR)/$(LIBGAME_DIR)
LIBGAME_HEADLESS_DIR = $(BUILD_HEADLESS_DIR)/$(LIBGAME_DIR)
SRCS_LIBGAME := 	system.c					\
					gadgets.c					\
					text.c						\
					sound.c						\
					snapshot.c					\
					joystick.c					\
					image.c						\
					random.c					\
					hash.c						\
					setup.c						\
					misc.c						\
					sdl.c						\
					zip/ioapi.c					\
					zip/iowin32.c				\
					zip/unzip.c					\
					zip/miniunz.c
OBJS_LIBGAME := $(SRCS_LIBGAME:.c=.o)

GAME_EM_DIR = 		game_em
GAME_EM_CUSTOM_DIR = $(BUILD_CUSTOM_DIR)/$(GAME_EM_DIR)
GAME_EM_HEADLESS_DIR = $(BUILD_HEADLESS_DIR)/$(GAME_EM_DIR)
SRCS_GAME_EM := 	cave.c						\
					convert.c					\
					graphics.c					\
					init.c						\
					input.c						\
					main.c						\
					sound.c						\
					synchro_1.c					\
					synchro_2.c					\
					synchro_3.c					\
					tab_generate.c				\
					ulaw_generate.c
OBJS_GAME_EM := $(SRCS_GAME_EM:.c=.o)

GAME_MM_DIR = 		game_mm
GAME_MM_CUSTOM_DIR = $(BUILD_CUSTOM_DIR)/$(GAME_MM_DIR)
GAME_MM_HEADLESS_DIR = $(BUILD_HEADLESS_DIR)/$(GAME_MM_DIR)
SRCS_GAME_MM := 	mm_init.c					\
					mm_main.c					\
					mm_game.c					\
					mm_files.c					\
					mm_tools.c
OBJS_GAME_MM := $(SRCS_GAME_MM:.c=.o)

GAME_SP_DIR = 		game_sp
GAME_SP_CUSTOM_DIR = $(BUILD_CUSTOM_DIR)/$(GAME_SP_DIR)
GAME_SP_HEADLESS_DIR = $(BUILD_HEADLESS_DIR)/$(GAME_SP_DIR)
SRCS_GAME_SP := 	init.c						\
					file.c						\
					main.c						\
					vb_lib.c					\
					ASM.c						\
					BugsTerminals.c				\
					DDScrollBuffer.c			\
					DDSpriteBuffer.c			\
					Display.c					\
					DoGameStuff.c				\
					Electrons.c					\
					Explosions.c				\
					Globals.c					\
					Infotrons.c					\
					InitGameConditions.c		\
					Input.c						\
					MainForm.c					\
					MainGameLoop.c				\
					Murphy.c					\
					OrangeDisk.c				\
					SnikSnaks.c					\
					Sound.c						\
					Zonk.c
OBJS_GAME_SP := $(SRCS_GAME_SP:.c=.o)

AI_DIR 			= 	ai
AI_CTR_DIR 		= 	controller
AI_BFS_DIR 		= 	$(AI_CTR_DIR)/bfs
AI_MCTS_DIR 	= 	$(AI_CTR_DIR)/mcts
AI_OPTION_DIR 	= 	$(AI_CTR_DIR)/options
AI_PFA_DIR 		= 	$(AI_CTR_DIR)/pfa
AI_REPLAY_DIR 	= 	$(AI_CTR_DIR)/replay
AI_USER_DIR 	= 	$(AI_CTR_DIR)/user
AI_ENG_DIR 		= 	engine
AI_UTIL_DIR 	=	util
AI_TEST_DIR 	=	tests
AI_CUSTOM_DIR 	= 	$(BUILD_CUSTOM_DIR)/$(AI_DIR)
AI_HEADLESS_DIR = 	$(BUILD_HEADLESS_DIR)/$(AI_DIR)
SRCS_AI 		= 	ai_entry.cpp 									\
					hash_table.cpp 									\
					$(AI_CTR_DIR)/controller.cpp 					\
					$(AI_MCTS_DIR)/mcts.cpp 						\
					$(AI_MCTS_DIR)/tree_node.cpp	  				\
					$(AI_OPTION_DIR)/option_factory.cpp 			\
					$(AI_OPTION_DIR)/base_option.cpp 				\
					$(AI_OPTION_DIR)/option_single_step.cpp 		\
					$(AI_OPTION_DIR)/option_to_sprite.cpp 			\
					$(AI_OPTION_DIR)/option_to_rock.cpp 			\
					$(AI_OPTION_DIR)/option_wait_rock.cpp			\
					$(AI_OPTION_DIR)/option_push_rock.cpp 			\
					$(AI_REPLAY_DIR)/replay.cpp 	 				\
					$(AI_ENG_DIR)/action.cpp 						\
					$(AI_ENG_DIR)/engine_helper.cpp 				\
					$(AI_ENG_DIR)/game_state.cpp					\
					$(AI_UTIL_DIR)/timer.cpp 						\
					$(AI_UTIL_DIR)/rng.cpp 							\
					$(AI_UTIL_DIR)/logging_wrapper.cpp 				\
					$(AI_UTIL_DIR)/level_programming.cpp  			\
					$(AI_UTIL_DIR)/summary_window.cpp 				\
					$(AI_TEST_DIR)/test_engine_speed.cpp 			\
					$(AI_TEST_DIR)/test_rng.cpp 
# 					$(AI_BFS_DIR)/bfs.cpp 							\
# 					$(AI_BFS_DIR)/tree_node_bfs.cpp 
# 					$(AI_USER_DIR)/user.cpp 						\
# 					$(AI_PFA_DIR)/pfa_mcts.cpp 						\
# 					$(AI_PFA_DIR)/pfa_tree_node.cpp 				\
# 					$(AI_PFA_DIR)/pfa.cpp 							\
# 					$(AI_PFA_DIR)/abstract_graph.cpp 				\
# 					$(AI_PFA_DIR)/abstract_node.cpp 				\

OBJS_AI 		= 	$(SRCS_AI:.cpp=.o)



RNDLIBS_CUSTOM 	= 	$(addprefix $(BUILD_CUSTOM_DIR)/, $(OBJS)) 				\
					$(addprefix $(LIBGAME_CUSTOM_DIR)/, $(OBJS_LIBGAME)) 	\
					$(addprefix $(GAME_SP_CUSTOM_DIR)/, $(OBJS_GAME_SP)) 	\
					$(addprefix $(GAME_EM_CUSTOM_DIR)/, $(OBJS_GAME_EM)) 	\
					$(addprefix $(GAME_MM_CUSTOM_DIR)/, $(OBJS_GAME_MM)) 	\
					$(addprefix $(AI_CUSTOM_DIR)/, $(OBJS_AI))	

RNDLIBS_HEADLESS = 	$(addprefix $(BUILD_HEADLESS_DIR)/, $(OBJS)) 			\
					$(addprefix $(LIBGAME_HEADLESS_DIR)/, $(OBJS_LIBGAME)) 	\
					$(addprefix $(GAME_SP_HEADLESS_DIR)/, $(OBJS_GAME_SP)) 	\
					$(addprefix $(GAME_EM_HEADLESS_DIR)/, $(OBJS_GAME_EM)) 	\
					$(addprefix $(GAME_MM_HEADLESS_DIR)/, $(OBJS_GAME_MM))	\
					$(addprefix $(AI_HEADLESS_DIR)/, $(OBJS_AI))

ALL_OBJS := $(RNDLIBS_CUSTOM) $(RNDLIBS_HEADLESS)
DEPENDENCIES = $(ALL_OBJS:.o=.d)


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
	@echo "Creating build directories..."
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(GAME_EM_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(GAME_EM_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(GAME_MM_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(GAME_MM_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(GAME_SP_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(GAME_SP_DIR)  
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(LIBGAME_DIR)/zip
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(LIBGAME_DIR)/zip
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(AI_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(AI_DIR)/$(AI_CTR_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(AI_DIR)/$(AI_BFS_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(AI_DIR)/$(AI_MCTS_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(AI_DIR)/$(AI_OPTION_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(AI_DIR)/$(AI_PFA_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(AI_DIR)/$(AI_REPLAY_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(AI_DIR)/$(AI_USER_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(AI_DIR)/$(AI_ENG_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(AI_DIR)/$(AI_UTIL_DIR)
	@mkdir -p $(BUILD_CUSTOM_DIR)/$(AI_DIR)/$(AI_TEST_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(AI_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(AI_DIR)/$(AI_CTR_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(AI_DIR)/$(AI_BFS_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(AI_DIR)/$(AI_MCTS_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(AI_DIR)/$(AI_OPTION_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(AI_DIR)/$(AI_PFA_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(AI_DIR)/$(AI_REPLAY_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(AI_DIR)/$(AI_USER_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(AI_DIR)/$(AI_ENG_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(AI_DIR)/$(AI_UTIL_DIR)
	@mkdir -p $(BUILD_HEADLESS_DIR)/$(AI_DIR)/$(AI_TEST_DIR)


custom: rocksndiamonds rocksndiamonds_headless

rocksndiamonds: $(RNDLIBS_CUSTOM) $(TIMESTAMP_FILE) $(ICON)
	@$(CXX) $(PROFILING) $(ICON) $(RNDLIBS_CUSTOM) $(LDFLAGS) -o ../$@

rocksndiamonds_headless: $(RNDLIBS_HEADLESS) $(TIMESTAMP_FILE) $(ICON)
	@$(CXX) $(PROFILING) $(ICON) $(RNDLIBS_HEADLESS) $(LDFLAGS) -o ../$@


# Libgame
$(BUILD_CUSTOM_DIR)/$(LIBGAME_DIR)/%.o: $(LIBGAME_DIR)/%.c
	$(CC) $(PROFILING) $(CFLAGS) -c $< -o $@
$(BUILD_HEADLESS_DIR)/$(LIBGAME_DIR)/%.o: $(LIBGAME_DIR)/%.c
	$(CC) $(PROFILING) $(CFLAGS) -DHEADLESS -c $< -o $@

# Game EM
$(BUILD_CUSTOM_DIR)/$(GAME_EM_DIR)/%.o: $(GAME_EM_DIR)/%.c
	$(CC) $(PROFILING) $(CFLAGS) -c $< -o $@
$(BUILD_HEADLESS_DIR)/$(GAME_EM_DIR)/%.o: $(GAME_EM_DIR)/%.c
	$(CC) $(PROFILING) $(CFLAGS) -DHEADLESS -c $< -o $@

# Game MM
$(BUILD_CUSTOM_DIR)/$(GAME_MM_DIR)/%.o: $(GAME_MM_DIR)/%.c
	$(CC) $(PROFILING) $(CFLAGS) -c $< -o $@
$(BUILD_HEADLESS_DIR)/$(GAME_MM_DIR)/%.o: $(GAME_MM_DIR)/%.c
	$(CC) $(PROFILING) $(CFLAGS) -DHEADLESS -c $< -o $@

# Game SP
$(BUILD_CUSTOM_DIR)/$(GAME_SP_DIR)/%.o: $(GAME_SP_DIR)/%.c
	$(CC) $(PROFILING) $(CFLAGS) -c $< -o $@
$(BUILD_HEADLESS_DIR)/$(GAME_SP_DIR)/%.o: $(GAME_SP_DIR)/%.c
	$(CC) $(PROFILING) $(CFLAGS) -DHEADLESS -c $< -o $@

# AI
$(BUILD_CUSTOM_DIR)/$(AI_DIR)/%.o: $(AI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_HEADLESS_DIR)/$(AI_DIR)/%.o: $(AI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -DHEADLESS -c $< -o $@

# root
$(BUILD_CUSTOM_DIR)/%.o: %.c
	$(CC) $(PROFILING) $(CFLAGS) -c $< -o $@
$(BUILD_HEADLESS_DIR)/%.o: %.c
	$(CC) $(PROFILING) $(CFLAGS) -DHEADLESS -c $< -o $@

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

$(TIMESTAMP_FILE): $(SRCS) $(RNDLIBS_CUSTOM) $(RNDLIBS_HEADLESS)
	@echo '#define SOURCE_DATE_STRING "$(SOURCE_DATE_STRING)"' \
	> $(TIMESTAMP_FILE)

$(ICON):
	$(CONVERT) $(ICON_PATH)/*.png $(CONVERT_ICON_ARGS) $(ICONBASE).ico
	echo "$(ICONBASE) ICON $(ICONBASE).ico" | $(WINDRES) -o $(ICON)
	#	$(CONVERT) $(ICON32X32) $(CONVERT_ICON_ARGS) $(ICONBASE).ico

.c.o:
	$(CC) $(PROFILING) $(CFLAGS) -c $*.c

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

