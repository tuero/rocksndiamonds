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

#include <vector>
#include <string>
#include <map>

#include "../base_controller.h"

#include "tree_node.h"
#include "../options/base_option.h"
#include "../options/option_factory.h"

#include "../../engine/game_state.h"
#include "../../engine/engine_types.h"
#include "../../util/timer.h"


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
    // !TODO Maybe put dirs into another header
    const std::string mcts_config = "mcts.config";
    const std::string config_dir = "./src/ai/config/";

    // MCTS runtime configurations
    // These are fallback values in the event configuration file isn't loaded
    int maxTime_ = 18000;              // Time limit
    int maxIterationsDepth_ = 10;           // Node limit
    int numSimulations_ = 20;           // Node limit
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

    Pointer root = nullptr;
    GameState rootSavedState;
    BaseOption *currentOption_;
    BaseOption *nextOption_;
    Action actionToSend;
    bool optionStatusFlag_ = true;


    /*
     * Log the current MCTS stats.
     */
    void logCurrentStats();

    /*
     * Select the child node based on UCT.
     *
     * @param node The ndoe to select the child from.
     */
    TreeNode* selectPolicyUCT(TreeNode* node);

    /*
     * Select node which was visited the most during MCTS
     *
     * If multiple nodes have share the most frequent visit count, one will be chosen
     * among those randomly.
     *
     * @param node The node to select the child from.
     */
    TreeNode* selectMostVisitedChild(TreeNode* node);

    /*
     * Get the value for the current node.
     *
     * Assumes the engine is currently set to the state which the nodes represents, as 
     * engine functions will be called. The state will then save its value, so the value
     * can be queried later, even if the engine is not set to the state.
     */
    float getNodeValue();

    /*
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

    MCTS() {}

    MCTS(OptionFactoryType optionType) : BaseController(optionType) {}


    void handleLevelStart() override;
    
    /*
     * Set option the agent will now take, and reset the MCTS search tree.
     * 
     * Called when the currentOption is complete. currentOption is now set to 
     * nextOption, which is the option MCTS wants to do next. The MCTS search tree
     * is then reset to the state after the current option is complete.
     * 
     * @param currentOption Option which the agent gets to execute.
     * @param nextOption Planned option for the agent to take at the future state.
     */
    // void handleEmpty() override;
    Action getAction() override;


    /*
     * Continue to find the next option the agent should take.
     *
     * Called during every game tick. Controller is planning on the next 
     * future state while agent is conducting the current option. currentOption holds 
     * the option the agent is currently conducting. The option to be taken once 
     * current option is complete should be put into nextOption.
     *
     * @param currentOption Option which the agent gets to execute.
     * @param nextOption Planned option for the agent to take at the future state.
     */
    // Action run() override;
    void plan() override;

    std::string controllerDetailsToString() override;

};

#endif  //MCTS


