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

using namespace enginehelper;


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
        while (multiplier_ <= (uint64_t)availableOptions_.size()) {
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
    playerCells_[0] = {-1, -1};
    playerCells_[1] = {-1, -1};

    PLOGD_(logger::FileLogger) << "------------------------";
    highLevelSearch();
    logHighLevelPath();
    // logRestrictedSprites();
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
    hashPathTimesVisited.clear();
    restrictedCellsByOption_.clear();
    restrictedCellsByOptionCount_.clear();
    for (auto const & hash : allOptionPairHashes()) {
        restrictedCellsByOption_[hash] = {};
        restrictedCellsByOptionCount_[hash] = 0;
    }

    // Set initial levin node
    openLevinNodes_.clear(); 
    for (auto const & option : availableOptions_) {
        int numGem = elementproperty::getItemGemCount(gridinfo::getSpriteGridCell(option->getSpriteID()));
        bool hasDoor = elementproperty::isExit(gridinfo::getSpriteGridCell(option->getSpriteID()));
        openLevinNodes_.insert({optionPairHash(option, option), 0, 0, CombinatorialPartition(), numGem, hasDoor});
    }

    initializationForEveryLevelStart();
}


/**
 * Get the action from the controller.
 */
Action TwoLevelSearch::getAction() {
    // Check if we need to poll the next option. 
    if (optionStatusFlag_) {
        // Get next option and set restricted cells
        solutionIndex_ += 1;

        // We don't have any more options...
        if (solutionIndex_ >= (int)highlevelPlannedPath_.size()) {
            requestReset_ = true;
            return Action::noop;
        }

        previousOption_ = (solutionIndex_ == 0) ? highlevelPlannedPath_[solutionIndex_] : currentOption_;
        currentOption_ = highlevelPlannedPath_[solutionIndex_];
        optionStatusFlag_ = false;

        PLOGD_(logger::FileLogger) << "Next option to execute: " << currentOption_->toString();
        PLOGD_(logger::FileLogger) << "Option restrictions: ";
        for (auto const & restriction : currentOption_->getRestrictedCells()) {
            PLOGD_(logger::FileLogger) << "x=" << restriction.x << ", y=" << restriction.y;
        }

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