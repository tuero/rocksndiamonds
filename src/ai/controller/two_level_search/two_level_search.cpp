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


void TwoLevelSearch::initializeOptions() {
    availableOptions_ = optionFactory_.createOptions(optionFactoryType_);

    multiplier_ = 10;
    while (multiplier_ < (uint64_t)availableOptions_.size()) {
        multiplier_ *= 10;
    }
    PLOGI_(logger::FileLogger) << "Multiplier used for hashing: " << multiplier_;
    PLOGI_(logger::ConsoleLogger) << "Multiplier used for hashing: " << multiplier_;
}


/**
 * 
 */
void TwoLevelSearch::resetOptions() {
    if (highlevelPlannedPath_.empty()) {
        availableOptions_ = optionFactory_.createOptions(optionFactoryType_);

        multiplier_ = 10;
        while (multiplier_ < (uint64_t)availableOptions_.size()) {
            multiplier_ *= 10;
        }
        PLOGI_(logger::FileLogger) << "Multiplier used for hashing: " << multiplier_;
        PLOGI_(logger::ConsoleLogger) << "Multiplier used for hashing: " << multiplier_;
    }
}


/**
 * Handle necessary items before the level gets restarted.
 */
void TwoLevelSearch::handleLevelRestartBefore() {
    
}


/**
 * Initializations which need to occur BOTH on first level start
 * and on every level restart after a failure.
 */
void TwoLevelSearch::initializationForEveryLevelStart() {
    requestReset_ = false;
    solutionIndex_ = -1;
    optionStatusFlag_ = true;

    // Player tracking for constraint detection
    prevPlayerCell_ = {-1, -1};
    currPlayerCell_ = {-1, -1};
    prevIsMoving_.clear();
    currIsMoving_.clear();

    PLOGD_(logger::FileLogger) << "------------------------";
    highLevelSearch();
    logHighLevelPath();
    logRestrictedSprites();
}


/**
 * Handle necessary items after the level gets restarted.
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

    // Clear and intialize data structures
    newSpriteFoundFlag_ = true;
    newConstraintFoundFlag_ = true;
    hashPathTimesVisited.clear();
    currSprites_.clear();
    spritesMoved.clear();
    restrictedCellsByOption_.clear();
    knownConstraints_.clear();
    for (auto const & hash : allOptionPairHashes()) {
        spritesMoved[hash] = {};
        restrictedCellsByOption_[hash] = {};
        knownConstraints_[hash] = {};
    }

    initializationForEveryLevelStart();

    // Log high level macro option path
    // logHighLevelPath();

    // Log restricted sprites
    // logRestrictedSprites();
}


/**
 * Get the action from the controller.
 */
Action TwoLevelSearch::getAction() {
    // Set currentOption to the option we want to use
    // PLOGD_(logger::ConsoleLogger) << "Solution index: " << solutionIndex_  << ", solution size: "  << highlevelPlannedPath_.size();
    // PLOGD_(logger::FileLogger) << "Solution index: " << solutionIndex_  << ", solution size: "  << highlevelPlannedPath_.size();

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
        previousOption_ = (solutionIndex_ == 0) ? highlevelPlannedPath_[solutionIndex_] : currentOption_;
        currentOption_ = highlevelPlannedPath_[solutionIndex_];
        optionStatusFlag_ = false;

        PLOGD_(logger::FileLogger) << "Next option to execute: " << currentOption_->toString();
        PLOGD_(logger::ConsoleLogger) << "Next option to execute: " << currentOption_->toString();

        PLOGD_(logger::FileLogger) << "Option restrictions: ";
        PLOGD_(logger::ConsoleLogger) << "Option restrictions: ";
        for (auto const & restriction : currentOption_->getRestrictedCells()) {
            PLOGD_(logger::FileLogger) << "x=" << restriction.x << ", y=" << restriction.y;
            PLOGD_(logger::ConsoleLogger) << "x=" << restriction.x << ", y=" << restriction.y;
        }

        // Hash the planned path and save for future duplicate detection
        // saveCurrentPathHash();

        // If option is not valid, we cannot progress further, and so set request reset flag
        if (!currentOption_->isValid()) {
            PLOGD_(logger::FileLogger) << "Option invalid, requesting reset";
            PLOGD_(logger::ConsoleLogger) << "Option invalid, requesting reset";
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
 * Use this time to check for moved objects.
 */
void TwoLevelSearch::plan() {
    checkForMovedObjects();
}