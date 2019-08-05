

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------
CUSTOM 		:= 	custom.a

SRCS 		:=	custom.cpp 						\
				hash_table.cpp

OBJS 		:=	custom.o 						\
	  			hash_table.o 

CTR_DIR 	:= 	controller
CTR_OBJ 	:= 	$(CTR_DIR)/controller.o 

BFS_DIR 	:= 	$(CTR_DIR)/bfs
BFS_OBJ 	:= 	$(BFS_DIR)/bfs.o 				\
				$(BFS_DIR)/tree_node_bfs.o	

MCTS_DIR 	:= 	$(CTR_DIR)/mcts
MCTS_OBJ 	:= 	$(MCTS_DIR)/mcts.o 				\
				$(MCTS_DIR)/tree_node.o		

PFA_DIR 	:= 	$(CTR_DIR)/pfa
PFA_OBJ 	:= 	$(PFA_DIR)/abstract_graph.o 				\
				$(PFA_DIR)/abstract_node.o 					\
				$(PFA_DIR)/pfa.o

REPLAY_DIR 	:= 	$(CTR_DIR)/replay
REPLAY_OBJ 	:= 	$(REPLAY_DIR)/replay.o 		

USER_DIR 	:= 	$(CTR_DIR)/user
USER_OBJ 	:= 	$(USER_DIR)/user.o 			

ENG_DIR 	:= 	engine
ENG_OBJ 	:= 	$(ENG_DIR)/action.o 			\
				$(ENG_DIR)/engine_helper.o 		\
				$(ENG_DIR)/game_state.o	

UTIL_DIR 	:=	util
UTIL_OBJ 	:= 	$(UTIL_DIR)/timer.o 				\
				$(UTIL_DIR)/rng.o 					\
				$(UTIL_DIR)/logging_wrapper.o 		\
				$(UTIL_DIR)/level_programming.o

TEST_DIR 	:=	tests
TEST_OBJ 	:= 	$(TEST_DIR)/test_engine_speed.o 	\
				$(TEST_DIR)/test_rng.o  			


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

.PHONY: all clean controller eng util tests 

all: controller eng util tests $(CUSTOM)

$(CUSTOM): $(CTR_OBJ) $(BFS_OBJ) $(MCTS_OBJ) $(PFA_OBJ) $(REPLAY_OBJ) $(USER_OBJ) $(ENG_OBJ) $(UTIL_OBJ) $(TEST_OBJ) $(OBJS)
	$(AR) cr $(CUSTOM) $(CTR_OBJ) $(BFS_OBJ) $(MCTS_OBJ) $(PFA_OBJ) $(REPLAY_OBJ) $(USER_OBJ) $(ENG_OBJ) $(UTIL_OBJ) $(TEST_OBJ) $(OBJS)
	$(RANLIB) $(CUSTOM)

controller:
	@$(MAKE) -C $(CTR_DIR)
$(CTR_OBJ):
	@$(MAKE) -C $(CTR_DIR)

eng:
	@$(MAKE) -C $(ENG_DIR)
$(ENG_OBJ):
	@$(MAKE) -C $(ENG_DIR)

util:
	@$(MAKE) -C $(UTIL_DIR)
$(UTIL_OBJ):
	@$(MAKE) -C $(UTIL_DIR)

tests:
	@$(MAKE) -C $(TEST_DIR)
$(TEST_OBJ):
	@$(MAKE) -C $(TEST_DIR)

%.o: %.cpp .depend
	$(CCP) $(CPPFLAGS) -c $*.cpp

clean:
	$(MAKE) -C $(CTR_DIR) clean
	$(MAKE) -C $(ENG_DIR) clean
	$(MAKE) -C $(UTIL_DIR) clean
	$(MAKE) -C $(TEST_DIR) clean
	$(RM) $(OBJS)
	$(RM) $(CUSTOM)
	$(RM) -f .depend

.depend: Makefile
	$(CCP) $(CPPFLAGS) -MM *.cpp $(DEP_OBJ) > .depend

-include .depend