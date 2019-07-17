

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS := 	mcts.cpp 		\
			tree_node.cpp

OBJS :=  	mcts.o			\
			tree_node.o



# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

.PHONY: all clean

all: $(OBJS)

%.o: %.cpp .depend
	$(CCP) $(PROFILING) $(CPPFLAGS) -c $*.cpp

clean:
	$(RM) $(OBJS)
	$(RM) -f .depend


.depend: Makefile
	$(CCP) $(CPPFLAGS) -MM *.cpp $(DEP_OBJ) > .depend

-include .depend

