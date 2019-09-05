

#ifndef PFAMCTS_H
#define PFAMCTS_H

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "pfa_tree_node.h"
#include "abstract_node.h"

#include "../../engine/game_state.h"
#include "../../engine/engine_types.h"
#include "../../engine/engine_helper.h"
#include "../../engine/action.h"
#include "../../util/timer.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


class PFA_MCTS {
    typedef std::unique_ptr<PFATreeNode> Pointer;
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
    int max_depth = 0;

    // Misc
    unsigned int calls_since_rest = 0;
    std::string msg;

    Pointer root = nullptr;
    GameState rootSavedState;
    AbstractNode* current_abstract_node_;
    AbstractNode* goal_abstract_node_;
    std::vector<enginetype::GridCell> allowed_cells_;
    std::vector<enginetype::GridCell> goal_cells_;

    void logCurrentStats();

    void logCurrentState(std::string msg, bool send_to_consol);

    PFATreeNode* selectBestChild(PFATreeNode* current);

    PFATreeNode* selectMostVisitedChild(PFATreeNode* current);

    float nodeValue(PFATreeNode* current);

    PFATreeNode* get_best_uct_child(PFATreeNode* node);

    PFATreeNode* getBestChild(PFATreeNode* current);

    std::string childValues(PFATreeNode* current);

public:

    PFA_MCTS();

    void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        AbstractNode* current_abstract_node, AbstractNode* goal_abstract_node);

    void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::deque<AbstractNode*> abstract_path, AbstractNode* goal_abstract_node);

    void reset(std::vector<Action> &next_action);

    void run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics);    

    enginetype::GridCell getRootPlayerCell();

};

#endif  //PFA_MCTS_H


