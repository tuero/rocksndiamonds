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

MAKE = make

SRC_DIR = src
MAKE_CMD = @$(MAKE) -C $(SRC_DIR)


all:
	$(MAKE_CMD)

clean:
	$(MAKE_CMD) clean

backup:
	./Scripts/make_backup.sh

dist-unix:
	./Scripts/make_dist_unix.sh .

dist-dos:
	./Scripts/make_dist_dos.sh .

dist: dist-unix dist-dos

depend:
	$(MAKE_CMD) depend
