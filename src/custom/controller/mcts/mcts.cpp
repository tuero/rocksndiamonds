
#include "mcts.h"

#include <iostream>
#include <fstream>
#include <algorithm>

// Engine
#include "../../engine/engine_types.h"
#include "../../engine/engine_helper.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


MCTS::MCTS(){
    // Load parameters from file
    std::string line;
    std::string parameter_name;
    float parameter_value;

    msg = "Cannot open mcts configuration file. Falling back to default parameters";

    try{
        std::ifstream configFile (config_dir + mcts_config);
        std::vector<Action> path;
        while (std::getline(configFile,line)){
            std::istringstream iss(line);
            if (!(iss >> parameter_name >> parameter_value)) {
                PLOGE_(logwrap::FileLogger) << "Bad line in config file, skipping.";
                continue;
            }

            // Process parameter
            if (parameter_name == "max_time") {
                maxTime_ = parameter_value;
            }
            else if (parameter_name == "max_iterations_depth") {
                max_iterations_depth = parameter_value;
            }
            else if (parameter_name == "num_simulations") {
                numSimulations_ = parameter_value;
            }
        }

        configFile.close();
    }
    catch (const std::ifstream::failure& e) {
        PLOGE_(logwrap::FileLogger) << "Cannot open file.";
    }

    timer.setLimit(maxTime_);

    // Log parameters being used
    PLOGD_(logwrap::FileLogger) << "MCTS parametrs...";
    PLOGD_(logwrap::FileLogger) << "max_time: " << maxTime_;
    PLOGD_(logwrap::FileLogger) << "max_iterations_depth: " << max_iterations_depth;
    PLOGD_(logwrap::FileLogger) << "num_simulations: " << numSimulations_;
}


void MCTS::logCurrentStats() {
    PLOGD_(logwrap::FileLogger) << "Time remaining: " << timer.getTimeLeft();
    PLOGD_(logwrap::FileLogger) << "Current number of expanded nodes: " << countExpandedNodes_ 
        << ", simulated nodes: " << countSimulatedNodes_;
}


void MCTS::logCurrentState(std::string msg, bool sendToConsol) {
    PLOGD_(logwrap::FileLogger) << msg;
    PLOGD_IF_(logwrap::ConsolLogger, sendToConsol) << msg;

    logwrap::logPlayerDetails();
    logwrap::logBoardState();
    logwrap::logMovPosState();
}


TreeNode* MCTS::selectPolicyUCT(TreeNode* node) {
    float UCT_K = sqrt(2);
    float epsilon = 0.00001;

    float bestScoreUCT = std::numeric_limits<float>::lowest();
    TreeNode* bestNode = NULL;

    // iterate all immediate children and find best UTC score
    for(std::size_t i = 0; i < node->getChildCount(); i++) {
        TreeNode* child = node->getChild(i);
        float childValue = child->getValue();
        float childVisitCount = (float)child->getVisitCount();

        float uct_exploitation = childValue / (childVisitCount + epsilon);
        float uct_exploration = sqrt(log(childVisitCount + 1) / (child->getVisitCount() + epsilon));
        float uct_score = uct_exploitation + UCT_K * uct_exploration;

        if(uct_score > bestScoreUCT) {
            bestScoreUCT = uct_score;
            bestNode = child;
        }
    }

    return bestNode;
}


TreeNode* MCTS::selectMostVisitedChild(TreeNode* current) {
    int mostVisitedCount = -1;
    TreeNode* mostVisitedChild = nullptr;

    for (std::size_t i = 0; i < current->getChildCount(); i++) {
        TreeNode* child = current->getChild(i);
        // Better child node found
        if (child->getVisitCount() > mostVisitedCount) {
            mostVisitedCount = child->getVisitCount();
            mostVisitedChild = child;
        }
    }
    return mostVisitedChild;
}


float MCTS::getNodeValue() {
    return enginehelper::getCurrentScore() - rootSavedState.getScore();
}



std::string MCTS::childValues(TreeNode* current) {
    std::string output = "";

    for (std::size_t i = 0; i < current->getChildCount(); i++) {
        TreeNode* childNode = current->getChild(i);
        int visitCount = childNode->getVisitCount();
        float value = childNode->getValue() / (float)visitCount;

        output += "\taction: ";
        output += actionToString(childNode->getActionTaken());
        output += ", value: ";
        output += std::to_string(value);
        output += ", visit count: ";
        output += std::to_string(visitCount);
        output += "\n";
    }

    return output;
}


void MCTS::reset(std::vector<Action> &next_action) {    
    // Reset statistics
    callsSinceReset_ = 0;
    countSimulatedNodes_ = 0;
    countExpandedNodes_ = 0;
    maxDepth_ = 0;

    // Save current state
    GameState reference_state;
    reference_state.setFromEngineState();

    // step forward
    enginehelper::setSimulatorFlag(true);
    std::string msg = "Setting root to next action in queue: ";
    msg += actionToString(next_action[0]);
    for (unsigned int i = 0; i < next_action.size(); i++) {
        enginehelper::setEnginePlayerAction(next_action[i]);
        enginehelper::engineSimulateSingle();
    }

    logCurrentState(msg, true);

    // save root state to where we will be when done executing current queued action
    root = std::make_unique<TreeNode>(nullptr);
    rootSavedState.setFromEngineState();
    root.get()->setActions();
    // TreeNode root(nullptr);
    // if (root == nullptr) {
    //     root = std::make_unique<TreeNode>(nullptr);
    // }
    // else {
    //     root = root->getChildByAction(next_action[0]);
    //     root.get()->setParent(nullptr);
    // }

    // reset engine back to reference state
    reference_state.restoreEngineState();
    enginehelper::setSimulatorFlag(false);
}


void MCTS::handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) {
    currentSolution = forwardSolution;
    PLOGD_(logwrap::FileLogger) << "Resetting MCTS tree.";
    PLOGD_(logwrap::ConsolLogger) << "Resetting MCTS tree.";
    reset(currentSolution);
}


void MCTS::run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) 
{
    // Silent compiler warning
    (void)currentSolution;

    // Set simulator flag, allows for optimized simulations
    enginehelper::setSimulatorFlag(true);

    // Timer gets initialized
    timer.reset();
    float loopCounter = 0;
    callsSinceReset_ += 1;

    // Save the current game state from the simulator
    GameState startingState;
    startingState.setFromEngineState();

    // Set current game state to root state
    rootSavedState.restoreEngineState();

    // MCTS starting configurations
    TreeNode* bestChild = nullptr;

    // Log state before MCTS to ensure we are simulating after queued actions are taken.
    // This allows for a next step partial solution to be given immediately after current
    // actions are finished.
    logCurrentState("State before MCTS.", false);

    forwardSolution.clear();
    timer.start();

    // Always loop until breaking conditions
    while (true) {
        // Step 1: Selection
        // Starting at root, continuously select best child using policy until leaf node
        // is met. A leaf is any node which no simulation has been initiated
        TreeNode* current = root.get();
        int currentDepth = 0;

        // Put simulator back to root state
        // !TODO -> Maybe encorporate set seed into restore simulator?
        rootSavedState.restoreEngineState();
        logCurrentStats();

        // Select child based on policy and forward engine simulator
        while (!current->getTerminalStatusFromEngine() && current->allExpanded()) {
            current = selectPolicyUCT(current);
            enginehelper::setEnginePlayerAction(current->getActionTaken());
            enginehelper::engineSimulate();
            currentDepth += 1;
        }

        maxDepth_ = std::max(maxDepth_, currentDepth);

        // Step 2: Expansion
        // We exand the node if its not terminal and not already fully expanded
        // Need to consider if state is winning state
        if (!current->getTerminalStatusFromEngine() && !current->allExpanded()) {
            current = current->expand();
            countExpandedNodes_ += 1;
        }

        // Before we simulate, we need to save a reference state to get back to
        GameState reference_state;
        reference_state.setFromEngineState();

        // Step 3: Simulation
        // The expansion step sets the simulator to the expanded node's state
        for (int i = 0; i < numSimulations_; i++) {
            reference_state.restoreEngineState();
            if (!current->isTerminal()) {
                for (int t = 0; t < max_iterations_depth; t++) {
                    // Terminal condition in simulator
                    if (enginehelper::engineGameFailed() || enginehelper::engineGameSolved()) {
                        break;
                    }

                    // Apply random action
                    enginehelper::setEngineRandomPlayerAction();
                    enginehelper::engineSimulate();
                    countSimulatedNodes_ += 1;
                }
            }
        }

        reference_state.restoreEngineState();
        float value = getNodeValue();

        // Step 4: Backpropagation   
        while (current != nullptr) {
            current->updateStats(value);
            current = current->getParent();
        }

        // Exit conditions: Reached max iterations or max time
        loopCounter += 1;
        float avgTime = timer.getDuration() / loopCounter;
        if (timer.getTimeLeft() < avgTime) {break;}
    }

    timer.stop();
    int ms = timer.getDuration();

    // Update statistics
    statistics[enginetype::RUN_TIME] = ms;
    statistics[enginetype::COUNT_EXPANDED_NODES] = countExpandedNodes_;
    statistics[enginetype::COUNT_SIMULATED_NODES] = countSimulatedNodes_;
    statistics[enginetype::MAX_DEPTH] = maxDepth_;

    // Get best child and its associated action
    rootSavedState.restoreEngineState();
    bestChild = selectMostVisitedChild(root.get());
    Action bestAction = (bestChild == nullptr ? Action::noop : bestChild->getActionTaken());

    // Logg root child nodes along with their current valuation
    msg = "Child node values:\n" + childValues(root.get());
    PLOGD_(logwrap::FileLogger) << msg;
    PLOGD_IF_(logwrap::ConsolLogger, callsSinceReset_ == 8) << msg;

    // Put simulator back to original state
    startingState.restoreEngineState();
    enginehelper::setSimulatorFlag(false);

    // return best action
    forwardSolution.clear();
    for (int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        forwardSolution.push_back(bestAction);
    }
}