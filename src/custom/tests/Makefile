

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS := 	test_engine_speed.cpp  	     		 	\
			test_rng.cpp 							
OBJS :=  	test_engine_speed.o 					\
			test_rng.o 								



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

