

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS := 	option_factory.cpp 					\
			base_option.cpp 					\
			option_single_step.cpp 				\
			option_to_sprite.cpp 				\
			option_to_rock.cpp 					\
			option_wait_rock.cpp 				\
			option_push_rock.cpp

OBJS :=  	option_factory.o 					\
			base_option.o 						\
			option_single_step.o 				\
			option_to_sprite.o					\
			option_to_rock.o					\
			option_wait_rock.o					\
			option_push_rock.o


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

