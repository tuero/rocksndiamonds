#=============================================================================#
# Makefile for Rocks'n'Diamonds 1.2                                           #
# (c) 1995-98 Holger Schemel, aeglos@valinor.owl.de                           #
#=============================================================================#

#-----------------------------------------------------------------------------#
# configuration section                                                       #
#-----------------------------------------------------------------------------#

#-----------------------------------------------------------------------------#
# you shouldn't need to change anything below                                 #
#-----------------------------------------------------------------------------#

PROGNAME = rocksndiamonds
SRC_DIR = src

MAKE = make
RM = rm -f
MV = mv -f

MAKE_CMD = $(MAKE) -C $(SRC_DIR)


all:
	$(MAKE_CMD)

clean:
	$(MAKE_CMD) clean

backup:
	./Scripts/make_backup.sh

dist: dist-unix dist-dos

dist-unix:
	./Scripts/make_dist_unix.sh .

dist-dos:
	./Scripts/make_dist_dos.sh .

depend:
	$(MAKE_CMD) depend
