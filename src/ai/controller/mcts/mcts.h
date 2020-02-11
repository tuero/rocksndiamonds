/**
 * @file: mcts.h
 *
 * @brief: MCTS controller which plans over options.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef MCTS_H
#define MCTS_H

// Standard Libary/STL
#include <vector>
#include <string>
#include <map>

// MCTS
#include "tree_node.h"
#include "game_state.h"

// Includes
#include "base_controller.h"
#include "base_option.h"
#include "option_factory.h"
#include "engine_types.h"
#include "timer.h"



/**
 * MCTS controller
 *
 * MCTS plans on the start after the agents current action is complete, so that 
 * another action is available immediately completing the current action.
 */
class MCTS : public BaseController {
    typedef std::unique_ptr<TreeNode> Pointer;
private:
    // Configuration file location
    const std::string MCTS_CONFIG_FILE = "mcts.config";

    // MCTS state values
    Pointer root = nullptr;                 // Pointer to root node (future state)
    GameState rootSavedState;               // Game state representing the root
    BaseOption *currentOption_;             // Current option which actions are being sent to engine
    BaseOption *nextOption_;                // The next option to execute (best as chosen by MCTS)
    bool optionStatusFlag_ = true;          // Flag to indicate the current option is finished and the next option should start

    // MCTS runtime configurations
    // These are fallback values in the event configuration file isn't loaded
    int maxTime_ = 18000;                   // Time limit
    int maxIterationsDepth_ = 5;            // Max depth during simulation
    int numSimulations_ = 5;                // Number of simulations to average the value from
    std::map<std::string, int*> configParameters_ = {
        {"max_time", &maxTime_},
        {"max_iterations_depth", &maxIterationsDepth_},
        {"num_simulations", &numSimulations_} 
    };

    // Statistics
    int countSimulatedNodes_ = 0;
    int countExpandedNodes_ = 0;
    int maxDepth_ = 0;

    // Misc
    int callsSinceReset_ = 0;
    std::string msg;


    /**
     * Log the current MCTS stats.
     */
    void logCurrentStats();

    /**
     * Set the MCTS parameters from the config file.
     */
    void setParamsFromConfig();

    /**
     * Select the child node based on UCT.
     *
     * @param node The ndoe to select the child from.
     */
    TreeNode* selectPolicyUCT(TreeNode* node);

    /**
     * Select node which was visited the most during MCTS
     *
     * If multiple nodes have share the most frequent visit count, one will be chosen
     * among those randomly.
     *
     * @param node The node to select the child from.
     */
    TreeNode* selectMostVisitedChild(TreeNode* node);

    /**
     * Get the value for the current node.
     *
     * Assumes the engine is currently set to the state which the nodes represents, as 
     * engine functions will be called. The state will then save its value, so the value
     * can be queried later, even if the engine is not set to the state.
     */
    double getNodeValue();

    /**
     * Reset the MCTS controller.
     *
     * MCTS search tree is reset to one state forward following nextOption. This lets us
     * search for the second option while we are currently executing the first option. By
     * default, the starting options is a single step noop.
     *
     * @param nextOption The planned next option, which the tree will be reset to.
     */
    void reset();


public:

    MCTS() {timer.setLimit(maxTime_);}

    MCTS(OptionFactoryType optionType) : BaseController(optionType) {}

    /**
     * Start of level handeling.
     */
    void handleLevelStart() override;
    
    /**
     * Get the next action to execture from the controller. Since the MCTS root
     * is at the state after execution, the sent action is from a saved option.
     * 
     * @return The action to execture.
     */
    Action getAction() override;


    /**
     * Continue to find the next option the agent should take.
     *
     * MCTS is run over the tree starting at the state AFTER the current option
     * is finished executing. To achieve real time, this step should take ~18ms.
     */
    void plan() override;

    /**
     * Controller details in readable format.
     */
    std::string controllerDetailsToString() override;

};

#endif  //MCTS


