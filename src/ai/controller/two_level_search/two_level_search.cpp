/**
 * @file: two_level_search.cpp
 *
 * @brief: Masters thesis controller.
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */


#include "two_level_search.h"

// Standard Libary/STL
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <bitset>

// Includes
#include "logger.h"

/**
 * Convey any important details about the controller in string format.
 */
std::string TwoLevelSearch::controllerDetailsToString() {
    return "Two level search controller.";
}


void TwoLevelSearch::resetOptions() {
    if (highlevelPlannedPath_.empty()) {
        availableOptions_ = optionFactory_.createOptions(optionFactoryType_);
    }
}


/**
 * Handle necessary items before the level gets restarted.
 */
void TwoLevelSearch::handleLevelRestartBefore() {
    // If we fail, we need to remember to save the state 
    // for the path we were just attempting
    AbstractGameState state;
    state.setFromEngineState();
    storedPathStates_[hash] = state;
}


void TwoLevelSearch::initializationForEveryLevelStart() {
    requestReset_ = false;
    solutionIndex_ = -1;
    optionStatusFlag_ = true;
    lowlevelPlannedPath_.clear();

    // Player tracking for constraint detection
    prevPlayerCell_ = {-1, -1};
    currPlayerCell_ = {-1, -1};
    prevIsMoving_.clear();
    currIsMoving_.clear();

    hash = 0;

    highLevelSearch();
}


/**
 * Handle necessary items after the level gets restarted.
 *  
 */
void TwoLevelSearch::handleLevelRestartAfter() {
    initializationForEveryLevelStart();
}


/**
 * Handle setup required at first level start.
 * 
 * Called only during level start. Any preprocessing or intiailizations needed for the 
 * controller that wouldn't otherwise be done during each game tick, should be setup here.
 */
void TwoLevelSearch::handleLevelStart() {
    logAvailableOptions();

    // Empty sprites 
    currSprites_.clear();
    spritesMoved.clear();
    for (auto const & option : availableOptions_) {
        spritesMoved[option] = {};
    }

    // Empty restrictions
    // !<TODO> rename to all constraints
    restrictedCellsByOption_.clear();
    for (auto const & option : availableOptions_) {
        restrictedCellsByOption_[option] = {};
    }

    knownConstraints_.clear();
    for (auto const & option : availableOptions_) {
        knownConstraints_[option] = {};
    }

    initializationForEveryLevelStart();

    // Log high level macro option path
    logHighLevelPath();

    // Log restricted sprites
    logRestrictedSprites();
}


/**
 * Get the action from the controller.
 */
Action TwoLevelSearch::getAction() {
    // Set currentOption to the option we want to use
    PLOGD_(logger::ConsoleLogger) << "Solution index: " << solutionIndex_  << ", solution size: "  << highlevelPlannedPath_.size();
    PLOGD_(logger::FileLogger) << "Solution index: " << solutionIndex_  << ", solution size: "  << highlevelPlannedPath_.size();

    // We don't have any more options...
    if (solutionIndex_ >= (int)highlevelPlannedPath_.size()) {
        PLOGE_(logger::FileLogger) << "No more options in solution list.";
        PLOGE_(logger::ConsoleLogger) << "No more options in solution list.";
        return Action::noop;
    }

    // Check if we need to poll the next option. 
    if (optionStatusFlag_) {
        // Get next option and set restricted cells
        solutionIndex_ += 1;
        currentOption_ = highlevelPlannedPath_[solutionIndex_];
        optionStatusFlag_ = false;

        PLOGI_(logger::FileLogger) << "Next option to execute: " << currentOption_->toString();
        PLOGI_(logger::ConsoleLogger) << "Next option to execute: " << currentOption_->toString();

        PLOGI_(logger::FileLogger) << "Option restrictions: ";
        PLOGI_(logger::ConsoleLogger) << "Option restrictions: ";
        for (auto const & restriction : currentOption_->getRestrictedCells()) {
            PLOGI_(logger::FileLogger) << "x=" << restriction.x << ", y=" << restriction.y;
            PLOGI_(logger::ConsoleLogger) << "x=" << restriction.x << ", y=" << restriction.y;
        }

        // Hash the planned path and save for future duplicate detection
        // saveCurrentPathHash();

        // If option is not valid, we cannot progress further, and so set request reset flag
        if (!currentOption_->isValid()) {
            PLOGI_(logger::FileLogger) << "Option invalid, requesting reset";
            PLOGI_(logger::ConsoleLogger) << "Option invalid, requesting reset";
            requestReset_ = true;
        }
    }

    // Get next option
    Action action = Action::noop;
    optionStatusFlag_ = true;
    requestReset_ = true;
    if (currentOption_->isValid()) {
        optionStatusFlag_ = currentOption_->getNextAction(action);
        requestReset_ = false;
    }

    return action;
}


/**
 * Continue to find the next option the agent should take.
 * 
 *
 * @param currentOption Option which the agent gets to execute.
 * @param nextOption Planned option for the agent to take at the future state.
 */
void TwoLevelSearch::plan() {
    checkForMovedObjects();
}