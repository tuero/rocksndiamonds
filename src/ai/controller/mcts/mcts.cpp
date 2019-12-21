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
#include <iostream>
#include <algorithm>
#include <random>
#include <cassert>

// Includes
#include "engine_types.h"
#include "engine_helper.h"
#include "../options/option_single_step.h"
#include "logger.h"


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


/*
 * Select the child node based on UCT.
 *
 * @param node The ndoe to select the child from.
 */
TreeNode* MCTS::selectPolicyUCT(TreeNode* node) {
    float UCT_K = sqrt(2);
    float epsilon = 0.00001;

    float bestScoreUCT = std::numeric_limits<float>::lowest();
    std::vector<TreeNode*> bestNodes;

    // iterate all immediate children and find best UTC score
    assert(node->getChildCount() > 0);
    for(std::size_t i = 0; i < node->getChildCount(); i++) {
        TreeNode* child = node->getChild(i);
        float childValue = child->getValue();
        float childVisitCount = (float)child->getVisitCount();

        float uct_exploitation = childValue / (childVisitCount + epsilon);
        float uct_exploration = sqrt(log(childVisitCount + 1) / (child->getVisitCount() + epsilon));
        float uct_score = uct_exploitation + UCT_K * uct_exploration;

        if(uct_score > bestScoreUCT) {
            bestScoreUCT = uct_score;
            bestNodes.clear();
            bestNodes.push_back(child);
        }
        else if (uct_score == bestScoreUCT) {
            bestNodes.push_back(child);
        }
    }

    std::shuffle(bestNodes.begin(), bestNodes.end(), g);

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

    for (std::size_t i = 0; i < node->getChildCount(); i++) {
        TreeNode* child = node->getChild(i);
        // Better count found
        if (child->getVisitCount() > mostVisitedCount) {
            mostVisitedCount = child->getVisitCount();
            mostVisitedChildren.clear();
        }

        if (child->getVisitCount() == mostVisitedCount) {
            mostVisitedChildren.push_back(child);
        }
    }

    std::sort(mostVisitedChildren.begin(), mostVisitedChildren.end(),
    [](const TreeNode* lhs, const TreeNode* rhs)
            { 
                 return (lhs->getValue() > rhs->getValue());
            }
    );  

    // std::shuffle(mostVisitedChildren.begin(), mostVisitedChildren.end(), g);
    return mostVisitedChildren[0];
}


/*
 * Get the value for the current node.
 *
 * Assumes the engine is currently set to the state which the nodes represents, as 
 * engine functions will be called. The state will then save its value, so the value
 * can be queried later, even if the engine is not set to the state.
 */
float MCTS::getNodeValue() {
    if (enginehelper::engineGameSolved()) {
        return enginehelper::getTimeLeftScore();
    }
    if (enginehelper::engineGameFailed()) {
        return -10;
    }
    return enginehelper::getCurrentScore() - rootSavedState.getScore() + enginehelper::countNumOfElement(enginetype::FIELD_DIAMOND);
}


/*
 * Stringify root child nodes with option, value, and visit count for logging.
 */
std::string MCTS::controllerDetailsToString() {
    std::string output = "";
    TreeNode* node = root.get();

    if (node == nullptr) {return output;}

    output += "Max depth: " + std::to_string(maxDepth_) + "\n";

    for (std::size_t i = 0; i < node->getChildCount(); i++) {
        TreeNode* childNode = node->getChild(i);
        int visitCount = childNode->getVisitCount();
        float value = childNode->getValue() / (float)visitCount;

        output += "\taction: ";
        output += childNode->getOptionTaken()->toString();
        output += ", value: ";
        output += std::to_string(value);
        output += ", visit count: ";
        output += std::to_string(visitCount);
        output += "\n";
    }

    return output;
}


void MCTS::handleLevelStart() {
    optionStatusFlag_ = true;
    nextOption_ = &noopOption;
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

    // Save current state
    GameState reference_state;
    reference_state.setFromEngineState();

    // step forward to state after current option
    enginehelper::setSimulatorFlag(true);
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

    currentOption_ = nextOption_;
    PLOGD_(logger::ConsoleLogger) << "Next option: " << currentOption_->toString() << " " << currentOption_->isValid();
    PLOGD_(logger::FileLogger) << "Next option: " << currentOption_->toString() << " " << currentOption_->isValid();

    logger::logCurrentState(plog::debug);
    logger::logBoardSpriteIDs(plog::debug);

    // save root state to where we will be when done executing current queued action
    root = std::make_unique<TreeNode>(nullptr);
    rootSavedState.setFromEngineState();
    root.get()->setOptions(availableOptions_);

    // reset engine back to reference state
    reference_state.restoreEngineState();
    enginehelper::setSimulatorFlag(false);
}


/*
 * Set option the agent will now take, and reset the MCTS search tree.
 * 
 * Called when the currentOption is complete. currentOption is now set to 
 * nextOption, which is the option MCTS wants to do next. The MCTS search tree
 * is then reset to the state after the current option is complete.
 */
Action MCTS::getAction() {
    // Ensure game is currently not over
    if (enginehelper::engineGameOver()) {
        return Action::noop;
    }

    // Check if option is complete
    if (optionStatusFlag_) {
        PLOGD_(logger::FileLogger) << "Resetting MCTS tree.";
        PLOGD_(logger::ConsoleLogger) << "Resetting MCTS tree.";
        reset();
    }

    Action action;
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
    if (enginehelper::engineGameOver() || rootSavedState.isGameOver()) {
        return;
    }

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
    // logCurrentState("State before MCTS.", false);

    // forwardSolution.clear();
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
        // <!-- Maybe use engine status rather than nodes?
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
        numSimulations_ = 5;
        maxIterationsDepth_ = 5;
        for (int i = 0; i < numSimulations_; i++) {
            reference_state.restoreEngineState();
            if (!current->isTerminal()) {
                for (int t = 0; t < maxIterationsDepth_; t++) {
                    // Terminal condition in simulator
                    if (enginehelper::engineGameFailed() || enginehelper::engineGameSolved()) {
                        break;
                    }

                    // Apply random action
                    // enginehelper::setEngineRandomPlayerAction();
                    // enginehelper::setEnginePlayerAction(current->getActionTaken());
                    // enginehelper::engineSimulate();
                    current->getOptionTaken()->run();
                    countSimulatedNodes_ += 1;
                }
            }
        }

        // reference_state.restoreEngineState();
        float value = getNodeValue();

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
    int ms = timer.getDuration();

    // Update statistics
    logCurrentStats();

    // Get best child and its associated action
    rootSavedState.restoreEngineState();
    bestChild = selectMostVisitedChild(root.get());
    // BaseOption* bestOption = bestChild->getOptionTaken();
    nextOption_ = bestChild->getOptionTaken();

    // Logg root child nodes along with their current valuation
    // msg = "Child node values:\n" + childrenToString(root.get());
    // PLOGD_(logwrap::FileLogger) << msg;
    // PLOGD_IF_(logwrap::ConsoleLogger, callsSinceReset_ == 8) << msg;
    // PLOGD_(logwrap::ConsoleLogger) << msg;

    // Put simulator back to original state
    startingState.restoreEngineState();
    enginehelper::setSimulatorFlag(false);

    // return best option
    // *nextOption = bestOption;
    // Action action;
    // optionStatusFlag_ = currentOption_->getNextAction(action);
    // return action;
}