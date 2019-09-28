

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

OPTION_DIR 	:= 	options
OPTION_OBJ 	:= 	$(OPTION_DIR)/option_factory.o 				\
				$(OPTION_DIR)/base_option.o 				\
				$(OPTION_DIR)/option_single_step.o 			\
				$(OPTION_DIR)/option_to_sprite.o 			\
				$(OPTION_DIR)/option_to_rock.o 				\
				$(OPTION_DIR)/option_wait_rock.o 			\
				$(OPTION_DIR)/option_push_rock.o 

PFA_DIR 	:= 	pfa
PFA_OBJ 	:= 	$(PFA_DIR)/abstract_graph.o 				\
				$(PFA_DIR)/abstract_node.o 					\
				$(PFA_DIR)/pfa_mcts.cpp 					\
				$(PFA_DIR)/pfa_tree_node.cpp 				\
				$(PFA_DIR)/pfa.o

REPLAY_DIR 	:= 	replay
REPLAY_OBJ 	:= 	$(REPLAY_DIR)/replay.o 		

USER_DIR 	:= 	user
USER_OBJ 	:= 	$(USER_DIR)/user.o 	



# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

.PHONY: all clean mcts option replay
	
all: mcts option replay $(OBJS)

bfs:
	@$(MAKE) -C $(BFS_DIR)
$(BFS_OBJ):
	@$(MAKE) -C $(BFS_DIR)

mcts:
	@$(MAKE) -C $(MCTS_DIR)
$(MCTS_OBJ):
	@$(MAKE) -C $(MCTS_DIR)

option:
	@$(MAKE) -C $(OPTION_DIR)
$(OPTION_OBJ):
	@$(MAKE) -C $(OPTION_DIR)

pfa:
	@$(MAKE) -C $(PFA_DIR)
$(PFA_OBJ):
	@$(MAKE) -C $(PFA_DIR)

replay:
	@$(MAKE) -C $(REPLAY_DIR)
$(REPLAY_OBJ):
	@$(MAKE) -C $(REPLAY_DIR)

user:
	@$(MAKE) -C $(USER_DIR)
$(USER_OBJ):
	@$(MAKE) -C $(USER_DIR)

%.o: %.cpp depend
	$(CCP) $(PROFILING) $(CPPFLAGS) -c $*.cpp

clean:
	$(MAKE) -C $(BFS_DIR) clean
	$(MAKE) -C $(MCTS_DIR) clean
	$(MAKE) -C $(OPTION_DIR) clean
	$(MAKE) -C $(PFA_DIR) clean
	$(MAKE) -C $(REPLAY_DIR) clean
	$(MAKE) -C $(USER_DIR) clean
	$(RM) $(OBJS)
	$(RM) -f .depend


depend: Makefile
	$(CCP)  $(CPPFLAGS) -MM *.cpp $(DEP_OBJ) > .depend

-include .depend

