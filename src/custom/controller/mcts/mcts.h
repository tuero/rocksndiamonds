

#ifndef MCTS_H
#define MCTS_H

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "../base_controller.h"

#include "tree_node.h"
#include "../../engine/game_state.h"
#include "../../engine/engine_types.h"
#include "../../engine/engine_helper.h"
#include "../../engine/action.h"
#include "../../util/timer.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


class MCTS : public BaseController {
    typedef std::unique_ptr<TreeNode> Pointer;
private:
    Timer timer;

    // Configuration file location
    // !TODO Maybe put dirs into another header
    const std::string mcts_config = "mcts.config";
    const std::string config_dir = "./src/custom/config/";

    // MCTS runtime configurations
    // These are fallback values in the event configuration file isn't loaded
    unsigned int max_time = 18000;              // Time limit
    unsigned int max_iterations_depth = 10;           // Node limit
    unsigned int num_simulations = 20;           // Node limit

    // Value function weights
    // These are fallback values in the event configuration file isn't loaded
    float w_distance = 7.0;
    float w_goals_count = 10.0;
    float w_died_count = 1.0;
    float w_safe_count = 1.0;
    float w_visit_count = 0.8;

    // Statistics
    unsigned int count_simulated_nodes = 0;
    unsigned int count_expanded_nodes = 0;
    unsigned int max_depth = 0;

    // Misc
    unsigned int calls_since_rest = 0;
    std::string msg;

    Pointer root = nullptr;
    GameState rootSavedState;

    TreeNode* selectBestChild(TreeNode* current);

    TreeNode* selectMostVisitedChild(TreeNode* current);

    float nodeValue(TreeNode* current);

    TreeNode* get_best_uct_child(TreeNode* node);

    std::string childValues(TreeNode* current);

public:

    MCTS();

    void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) override;

    void reset(std::vector<Action> &next_action);

    void run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) override;    

};

#endif  //MCTS


