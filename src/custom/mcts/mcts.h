

#ifndef MCTS_H
#define MCTS_H

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>


#include "tree_node.h"
#include "../engine/game_state.h"
#include "../engine/engine_helper.h"
#include "../engine/action.h"
#include "../util/timer.h"


class MCTS {
    typedef std::unique_ptr<TreeNode> Pointer;
private:
    Timer timer;
    unsigned int max_time;              // Time limit
    unsigned int max_iterations;        // Node limit (may not be used)
    int current_iterations;

    TreeNode* selectBestChild(TreeNode* current);

public:

    MCTS();


    Action run();    


};

#endif  //MCTS


