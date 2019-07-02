

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS := 	game_state.cpp 			\
			action.cpp 				\
			engine_helper.cpp

OBJS :=  	game_state.o 			\
			action.o 				\
			engine_helper.o



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

