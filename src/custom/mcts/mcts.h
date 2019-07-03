

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
    unsigned int max_time = 40000;            // Time limit
    unsigned int max_iterations = 20;        // Node limit (may not be used)
    unsigned int count_iterations;
    unsigned int count_nodes;

    TreeNode* selectBestChild(TreeNode* current);

    TreeNode* selectMostVisitedChild(TreeNode* current);

    float nodeValue(TreeNode* current);

public:

    MCTS();


    void run(std::vector<Action> &solution, int &total_time, int &iterations, int &nodes_expanded);    


};

#endif  //MCTS


