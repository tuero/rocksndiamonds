/**
 * @file: mcts.cpp
 *
 * @brief: MCTS controller which plans over options.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */


#include "mcts.h"

// Standard Libary/STL
#include <unordered_map>
#include <algorithm>
#include <random>

// Includes
#include "engine_helper.h"
#include "engine_types.h"
#include "config_reader.h"
#include "../options/option_single_step.h"
#include "logger.h"

using namespace enginehelper;


std::random_device rd;
std::mt19937 g(rd());
OptionSingleStep noopOption(Action::noop, 1);


/*
 * Log the current MCTS stats.
 */
void MCTS::logCurrentStats() {
    PLOGD_(logger::FileLogger) << "Time remaining: " << timer.getTimeRemaining();
    PLOGD_(logger::FileLogger) << "Current number of expanded nodes: " << countExpandedNodes_
                               << ", simulated nodes: " << countSimulatedNodes_
                               << ", max depth: " << maxDepth_;
}


/**
 * Set the MCTS parameters from the config file.
 */
void MCTS::setParamsFromConfig() {
    PLOGD_(logger::FileLogger) << "Reading values from MCTS configuration";
    std::unordered_map<std::string, double> configValues = config::getConfigValues(MCTS_CONFIG_FILE);

    for (auto const & configItem : configValues) {
        // Parameter in file matches known parameter
        if (configParameters_.find(configItem.first) != configParameters_.end()) {
            // Downcasting to int, which is fine here.
            *(configParameters_[configItem.first]) = (int)configItem.second;
        }
    }

    // Log configuration values being used
    PLOGD_(logger::FileLogger) << "MCTS config parameters being used:";
    PLOGD_(logger::ConsoleLogger) << "MCTS config parameters being used:";
    for (auto const & configItem : configParameters_) {
        PLOGD_(logger::FileLogger) << "\t" << configItem.first << ": " << *(configItem.second);
        PLOGD_(logger::ConsoleLogger) << "\t" << configItem.first << ": " << *(configItem.second);
    }
}


/*
 * Select the child node based on UCT.
 *
 * @param node The ndoe to select the child from.
 */
TreeNode* MCTS::selectPolicyUCT(TreeNode* node) {
    // UCT parameters
    float UCT_K = sqrt(2);
    float epsilon = 0.00001;

    float bestScoreUCT = std::numeric_limits<float>::lowest();
    std::vector<TreeNode*> bestNodes;

    // iterate all immediate children and find best UTC score
    for(std::size_t i = 0; i < node->getChildCount(); i++) {
        TreeNode* child = node->getChild(i);
        float childValue = child->getValue();
        float childVisitCount = (float)child->getVisitCount();

        // UCT values
        float uct_exploitation = childValue / (childVisitCount + epsilon);
        float uct_exploration = sqrt(log(childVisitCount + 1) / (child->getVisitCount() + epsilon));
        float uct_score = uct_exploitation + UCT_K * uct_exploration;

        // Better value found
        if(uct_score > bestScoreUCT) {
            bestScoreUCT = uct_score;
            bestNodes.clear();
        }

        // Add as candidate
        if (uct_score == bestScoreUCT) {
            bestNodes.push_back(child);
        }
    }

    // Shouldn't happen but safeguard against no children
    if (bestNodes.empty()) {
        PLOGE_(logger::FileLogger) << "No children available to select from.";
        PLOGE_(logger::ConsoleLogger) << "No children available to select from.";
        throw std::exception();
    }

    // Select randomly best node
    if (bestNodes.size() > 1) {
        std::shuffle(bestNodes.begin(), bestNodes.end(), g);
    }

    return bestNodes[0];
}


/*
 * Select node which was visited the most during MCTS
 *
 * If multiple nodes have share the most frequent visit count, one will be chosen
 * among those randomly.
 */
TreeNode* MCTS::selectMostVisitedChild(TreeNode* node) {
    int mostVisitedCount = -1;
    std::vector<TreeNode*> mostVisitedChildren;

    // Iterate children
    for (std::size_t i = 0; i < node->getChildCount(); i++) {
        TreeNode* child = node->getChild(i);

        // Better count found
        if (child->getVisitCount() > mostVisitedCount) {
            mostVisitedCount = child->getVisitCount();
            mostVisitedChildren.clear();
        }

        // Add as candidate
        if (child->getVisitCount() == mostVisitedCount) {
            mostVisitedChildren.push_back(child);
        }
    }

    // Select a best child, tiebreaking by better value
    std::sort(mostVisitedChildren.begin(), mostVisitedChildren.end(),
    [](const TreeNode* lhs, const TreeNode* rhs)
            { 
                 return (lhs->getValue() > rhs->getValue());
            }
    );  

    // Shouldn't happen but safeguard against no children
    if (mostVisitedChildren.empty()) {
        PLOGE_(logger::FileLogger) << "No children available to select from.";
        PLOGE_(logger::ConsoleLogger) << "No children available to select from.";
        throw std::exception();
    }

    return mostVisitedChildren[0];
}


/*
 * Get the value for the current node.
 *
 * Assumes the engine is currently set to the state which the nodes represents, as 
 * engine functions will be called. The state will then save its value, so the value
 * can be queried later, even if the engine is not set to the state.
 */
double MCTS::getNodeValue() {
    if (enginestate::engineGameSolved()) {
        return enginestate::getTimeLeftScore() > 999 ? enginestate::getTimeLeftScore() : 999;
    }
    if (enginestate::engineGameFailed()) {
        return -10;
    }
    return enginestate::getCurrentScore() - rootSavedState.getScore();
}


/*
 * Stringify root child nodes with option, value, and visit count for logging.
 */
std::string MCTS::controllerDetailsToString() {
    std::string output = "";
    TreeNode* node = root.get();

    if (node == nullptr) {return output;}

    output += "Max depth: " + std::to_string(maxDepth_) + "\n";

    // Log children details
    for (std::size_t i = 0; i < node->getChildCount(); i++) {
        TreeNode* childNode = node->getChild(i);
        int visitCount = childNode->getVisitCount();
        double value = childNode->getValue() / (double)visitCount;
        output  += "\taction: " + childNode->getOptionTaken()->toString() + ", value: "
                + std::to_string(value) + ", visit count: " + std::to_string(visitCount) + "\n";
    }

    return output;
}


/**
 * Start of level handeling
 */
void MCTS::handleLevelStart() {
    timer.setLimit(maxTime_);
    setParamsFromConfig();
    optionStatusFlag_ = true;
    nextOption_ = &noopOption;

    // Set options to try and avoid pathing into cells which cause an auto death (rock immediately above)
    for (auto const & option : availableOptions_) {
        option->setPrioritizeSafeCells(true);
    }
}


/*
 * Reset the MCTS controller.
 *
 * MCTS search tree is reset to one state forward following nextOption. This lets us
 * search for the second option while we are currently executing the first option. By
 * default, the starting options is a single step noop.
 */
void MCTS::reset() {   
    // Reset statistics
    callsSinceReset_ = 0;
    countSimulatedNodes_ = 0;
    countExpandedNodes_ = 0;
    maxDepth_ = 0;

    // Save current engine state
    GameState reference_state;
    reference_state.setFromEngineState();

    // step forward to state after current option
    enginestate::setSimulatorFlag(true);
    std::string msg = "Setting root to next option in queue: ";

    // Safeguard against bad pointers, option will be to step forward my performing noop action
    if (nextOption_ == nullptr) {
        OptionSingleStep tempOption = OptionSingleStep(Action::noop, 1);
        msg += tempOption.toString();
        tempOption.run();
    }
    else {
        msg += nextOption_->toString();
        nextOption_->run();
    }

    // Logging
    currentOption_ = nextOption_;
    PLOGD_(logger::ConsoleLogger) << "Next option: " << currentOption_->toString() << " " << currentOption_->isValid();
    PLOGD_(logger::FileLogger) << "Next option: " << currentOption_->toString() << " " << currentOption_->isValid();
    PLOGD_(logger::FileLogger) << controllerDetailsToString();

    // save root state to where we will be when done executing current queued action
    root = std::make_unique<TreeNode>(nullptr);
    rootSavedState.setFromEngineState();
    root.get()->setOptions(availableOptions_);

    // reset engine back to reference state
    reference_state.restoreEngineState();
    enginestate::setSimulatorFlag(false);
}


/*
 * Set option the agent will now take, and reset the MCTS search tree.
 * 
 * Called when the currentOption is complete. currentOption is now set to 
 * nextOption, which is the option MCTS wants to do next. The MCTS search tree
 * is then reset to the state after the current option is complete.
 */
Action MCTS::getAction() {
    Action action = Action::noop;

    // Ensure game is currently not over
    if (enginestate::engineGameOver()) {
        return action;
    }

    // Check if option is complete
    if (optionStatusFlag_) {
        PLOGD_(logger::FileLogger) << "Resetting MCTS tree.";
        PLOGD_(logger::ConsoleLogger) << "Resetting MCTS tree.";
        reset();
    }

    // Get next action from the current option 
    optionStatusFlag_ = currentOption_->getNextAction(action);
    return action;
}


/*
 * Continue to find the next option the agent should take.
 *
 * Called during every game tick. Controller is planning on the next 
 * future state while agent is conducting the current option. currentOption holds 
 * the option the agent is currently conducting. The option to be taken once 
 * current option is complete should be put into nextOption.
 */
void MCTS::plan() {
    // Break early if either current state or root state is already over.
    if (enginestate::engineGameOver() || rootSavedState.isGameOver()) {
        return;
    }

    // Set simulator flag, allows for optimized simulations
    enginestate::setSimulatorFlag(true);

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
    timer.start();

    // Main MCTS loop
    while (true) {
        // Step 1: Selection
        // Starting at root, continuously select best child using policy until leaf node
        // is met. A leaf is any node which no simulation has been initiated
        TreeNode* current = root.get();
        int currentDepth = 0;

        // Put simulator back to root state
        rootSavedState.restoreEngineState();
        logCurrentStats();

        // Select child based on policy and forward engine simulator
        while (!current->isTerminal() && current->allExpanded()) {
            current = selectPolicyUCT(current);
            current->getOptionTaken()->run();
            currentDepth += 1;
        }

        maxDepth_ = std::max(maxDepth_, currentDepth);

        // Step 2: Expansion
        // We exand the node if its not terminal and not already fully expanded
        if (!current->isTerminal() && !current->allExpanded()) {
            current = current->expand();
            countExpandedNodes_ += 1;
        }

        // Before we simulate, we need to save a reference state to get back to
        GameState reference_state;
        reference_state.setFromEngineState();

        // Step 3: Simulation
        // The expansion step sets the simulator to the expanded node's state
        double value = 0.0;
        for (int i = 0; i < numSimulations_; i++) {
            reference_state.restoreEngineState();
            if (!current->isTerminal()) {
                enginestate::setEngineRandomPlayerAction();
                Action action = enginestate::getEnginePlayerAction();
                for (int t = 0; t < maxIterationsDepth_; t++) {
                    if (enginestate::engineGameFailed() || enginestate::engineGameSolved()) {
                        break;
                    }

                    // Apply random action
                    enginestate::setEnginePlayerAction(action);
                    enginestate::engineSimulate();
                    countSimulatedNodes_ += 1;
                }

                // Get simulated value
                value += getNodeValue();
            }
        }

        // Average values seen
        value /= numSimulations_;

        // Step 4: Backpropagation   
        while (current != nullptr) {
            current->updateStats(value);
            current = current->getParent();
        }

        // Exit conditions: Reached max iterations or max time
        loopCounter += 1;
        float avgTime = timer.getDuration() / loopCounter;
        if (timer.getTimeRemaining() < avgTime) {break;}
    }

    timer.stop();

    // Update statistics
    logCurrentStats();

    // Get best child and its associated action
    rootSavedState.restoreEngineState();
    bestChild = selectMostVisitedChild(root.get());
    nextOption_ = bestChild->getOptionTaken();

    // Put simulator back to original state
    startingState.restoreEngineState();
    enginestate::setSimulatorFlag(false);
}