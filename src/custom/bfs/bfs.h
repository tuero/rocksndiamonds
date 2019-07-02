#ifndef BFS_H
#define BFS_H

// ------------- Includes  -------------
#include <memory>
#include <vector>
#include <algorithm>
#include <vector>
#include <queue>
#include <algorithm>

#include "tree_node_bfs.h"
#include "../engine/engine_helper.h"
#include "../engine/game_state.h"
#include "../engine/action.h"
#include "../hash_table.h"
#include "../util/timer.h"



void bfs(std::vector<Action> &solution);

#endif