#ifndef BFS_H
#define BFS_H

// ------------- Includes  -------------
#include <memory>
#include <vector>
#include <algorithm>
#include <vector>
#include <queue>
#include <algorithm>

#include "../base_controller.h"

//Logging
#include "../../util/logging_wrapper.h"
#include <plog/Log.h>   

#include "tree_node_bfs.h"
#include "../../engine/engine_helper.h"
#include "../../engine/game_state.h"
#include "../../engine/action.h"
#include "../../hash_table.h"
#include "../../util/timer.h"



class BFS : public BaseController {
private:
    unsigned int count_expanded_nodes;
public:

    BFS();

    void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) override;

    /*
     * Run BFS and return a solution
     */
    void run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) override;

    /*
     * Get the number of nodes expanded during BFS
     */
    unsigned int getCountNodes() const;
};

#endif