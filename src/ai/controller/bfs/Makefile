

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS :=  	bfs.cpp 					\
			tree_node_bfs.cpp

OBJS :=  	bfs.o 						\
			tree_node_bfs.o



# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

.PHONY: all clean

all: $(OBJS)

%.o: %.cpp depend
	$(CCP) $(PROFILING) $(CPPFLAGS) -c $*.cpp

clean:
	$(RM) $(OBJS)
	$(RM) -f .depend


depend: Makefile
	$(CCP)  $(CPPFLAGS) -MM *.cpp $(DEP_OBJ) > .depend

-include .depend

