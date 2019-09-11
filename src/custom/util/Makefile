

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS := 	timer.cpp 								\
			rng.cpp 								\
			logging_wrapper.cpp 					\
			level_programming.cpp 					\
			summary_window.cpp 

OBJS :=  	timer.o 								\
			rng.o 									\
			logging_wrapper.o 						\
			level_programming.o 					\
			summary_window.o



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

