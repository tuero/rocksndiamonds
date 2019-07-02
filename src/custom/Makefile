

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------
CUSTOM 		:= 	custom.a

SRCS 		:=	custom.cpp 						\
				hash_table.cpp

OBJS 		:=	custom.o 						\
	  			hash_table.o 

BFS_DIR 	:= 	bfs
BFS_OBJ 	:= 	$(BFS_DIR)/bfs.o 				\
				$(BFS_DIR)/tree_node_bfs.o	

ENG_DIR 	:= 	engine
ENG_OBJ 	:= 	$(ENG_DIR)/action.o 			\
				$(ENG_DIR)/engine_helper.o 		\
				$(ENG_DIR)/game_state.o	

MCTS_DIR 	:= 	mcts
MCTS_OBJ 	:= 	$(MCTS_DIR)/mcts.o 				\
				$(MCTS_DIR)/tree_node.o	

UTIL_DIR 	:=	util
UTIL_OBJ 	:= 	$(UTIL_DIR)/timer.o 	


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

.PHONY: all clean bfs eng mcts util

all: bfs eng mcts util $(CUSTOM)

$(CUSTOM): $(BFS_OBJ) $(ENG_OBJ) $(MCTS_OBJ) $(UTIL_OBJ) $(OBJS)
	$(AR) cr $(CUSTOM) $(BFS_OBJ) $(ENG_OBJ) $(MCTS_OBJ) $(UTIL_OBJ) $(OBJS)
	$(RANLIB) $(CUSTOM)

bfs:
	@$(MAKE) -C $(BFS_DIR)
$(BFS_OBJ):
	@$(MAKE) -C $(BFS_DIR)

eng:
	@$(MAKE) -C $(ENG_DIR)
$(ENG_OBJ):
	@$(MAKE) -C $(ENG_DIR)

mcts:
	@$(MAKE) -C $(MCTS_DIR)
$(MCTS_OBJ):
	@$(MAKE) -C $(MCTS_DIR)

util:
	@$(MAKE) -C $(UTIL_DIR)
$(UTIL_OBJ):
	@$(MAKE) -C $(UTIL_DIR)

%.o: %.cpp .depend
	$(CCP) $(CPPFLAGS) -c $*.cpp

clean:
	$(MAKE) -C $(BFS_DIR) clean
	$(MAKE) -C $(ENG_DIR) clean
	$(MAKE) -C $(MCTS_DIR) clean
	$(MAKE) -C $(UTIL_DIR) clean
	$(RM) $(OBJS)
	$(RM) $(CUSTOM)
	$(RM) -f .depend

.depend: Makefile
	$(CCP) $(CPPFLAGS) -MM *.cpp $(DEP_OBJ) > .depend

-include .depend