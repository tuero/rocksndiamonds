

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS := 	abstract_graph.cpp 		\
			abstract_node.cpp 		\
			pfa_mcts.cpp 			\
			pfa_tree_node.cpp 		\
			pfa.cpp

OBJS :=  	abstract_graph.o			\
			abstract_node.o 			\
			pfa_mcts.o 			\
			pfa_tree_node.o 		\
			pfa.o



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
	$(CCP)  $(CPPFLAGS) -MM *.cpp $(DEP_OBJ) > .depend

-include .depend

