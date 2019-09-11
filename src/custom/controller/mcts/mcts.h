

#ifndef MCTS_H
#define MCTS_H

#include <vector>
#include <string>
#include <map>

#include "../base_controller.h"

#include "tree_node.h"
#include "../../engine/game_state.h"
#include "../../engine/action.h"
#include "../../util/timer.h"


class MCTS : public BaseController {
    typedef std::unique_ptr<TreeNode> Pointer;
private:
    // Configuration file location
    // !TODO Maybe put dirs into another header
    const std::string mcts_config = "mcts.config";
    const std::string config_dir = "./src/custom/config/";

    // MCTS runtime configurations
    // These are fallback values in the event configuration file isn't loaded
    int maxTime_ = 18000;              // Time limit
    int max_iterations_depth = 10;           // Node limit
    int numSimulations_ = 20;           // Node limit

    // Statistics
    int countSimulatedNodes_ = 0;
    int countExpandedNodes_ = 0;
    int maxDepth_ = 0;

    // Misc
    int callsSinceReset_ = 0;
    std::string msg;

    Pointer root = nullptr;
    GameState rootSavedState;
    Timer timer;


    TreeNode* selectMostVisitedChild(TreeNode* current);

    float getNodeValue();

    TreeNode* selectPolicyUCT(TreeNode* node);

    std::string childValues(TreeNode* current);

    void logCurrentStats();

    void logCurrentState(std::string msg, bool sendToConsol);

public:

    MCTS();

    void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) override;

    void reset(std::vector<Action> &next_action);

    void run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) override;    

};

#endif  //MCTS


