

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS := 	controller.cpp

OBJS :=  	controller.o 

BFS_DIR 	:= 	bfs
BFS_OBJ 	:= 	$(BFS_DIR)/bfs.o 				\
				$(BFS_DIR)/tree_node_bfs.o	

MCTS_DIR 	:= 	mcts
MCTS_OBJ 	:= 	$(MCTS_DIR)/mcts.o 				\
				$(MCTS_DIR)/tree_node.o	

REPLAY_DIR 	:= 	replay
REPLAY_OBJ 	:= 	$(REPLAY_DIR)/replay.o 		

USER_DIR 	:= 	user
USER_OBJ 	:= 	$(USER_DIR)/user.o 	



# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

.PHONY: all clean bfs mcts replay user

all: bfs mcts replay user $(OBJS)

bfs:
	@$(MAKE) -C $(BFS_DIR)
$(BFS_OBJ):
	@$(MAKE) -C $(BFS_DIR)

mcts:
	@$(MAKE) -C $(MCTS_DIR)
$(MCTS_OBJ):
	@$(MAKE) -C $(MCTS_DIR)

replay:
	@$(MAKE) -C $(REPLAY_DIR)
$(REPLAY_OBJ):
	@$(MAKE) -C $(REPLAY_DIR)

user:
	@$(MAKE) -C $(USER_DIR)
$(USER_OBJ):
	@$(MAKE) -C $(USER_DIR)

%.o: %.cpp .depend
	$(CCP) $(PROFILING) $(CPPFLAGS) -c $*.cpp

clean:
	$(MAKE) -C $(BFS_DIR) clean
	$(MAKE) -C $(MCTS_DIR) clean
	$(MAKE) -C $(REPLAY_DIR) clean
	$(MAKE) -C $(USER_DIR) clean
	$(RM) $(OBJS)
	$(RM) -f .depend


.depend: Makefile
	$(CCP) $(CPPFLAGS) -MM *.cpp $(DEP_OBJ) > .depend

-include .depend

