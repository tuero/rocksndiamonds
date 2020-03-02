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
#include "util/tls_hash.h"
#include "statistics.h"
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
    PLOGD_(logger::FileLogger) << "Multiplier used for hashing: " << multiplier_;
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
        PLOGD_(logger::FileLogger) << "Multiplier used for hashing: " << multiplier_;
    }
}


/**
 * Handle necessary items before the level gets restarted.
 */
void TwoLevelSearch::handleLevelRestartBefore() {
    
}


void TwoLevelSearch::incrementPathTimesVisited() {
    // Statistics logging
    ++(statistics::pathCounts[levelinfo::getLevelNumber()][currentHighLevelPathHash_]);
    statistics::solutionPathCounts[levelinfo::getLevelNumber()][0] = currentHighLevelPathHash_;
    statistics::solutionPathCounts[levelinfo::getLevelNumber()][1] = ++hashPathTimesVisited[currentHighLevelPathHash_];
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
    incrementPathTimesVisited();
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

    initialState.setFromEngineState();

    // Clear and intialize data structures
    hashPathTimesVisited.clear();
    restrictedCellsByOption_.clear();
    restrictedCellsByPath_.clear();
    restrictedCellsByOptionCount_.clear();
    for (auto const & hash : tlshash::allItemsPairHashes(availableOptions_, multiplier_)) {
        restrictedCellsByOption_[hash] = {};
        restrictedCellsByOptionCount_[hash] = 0;
    }


// Manally adding constraints
#ifdef MANUAL_CONSTRAINTS
    // Find rocks (potential restrictions)
    std::vector<enginetype::GridCell> rockCells;
    PLOGE_(logger::FileLogger) << "Rocks";
    for (auto const & cell : gridinfo::getMapSprites()) {
        if (gridinfo::getGridElement(cell) == enginetype::ELEMENT_BD_ROCK) {
            rockCells.push_back({cell.x, cell.y + 1});
            PLOGE_(logger::FileLogger) <<  cell.x << "," << cell.y+1;
        }
    }

    auto addRestrictionsIfApplicable = [&](uint64_t hash, const std::deque<enginetype::GridCell> &solutionPath) {
        for (auto const & cell :solutionPath) {
            if (std::find(rockCells.begin(), rockCells.end(), cell) != rockCells.end()) {
                restrictedCellsByOption_[hash].insert(gridinfo::cellToIndex(cell));
                ++restrictedCellsByOptionCount_[hash];
                PLOGE_(logger::FileLogger) <<  cell.x << "," << cell.y+1;
            }
        }
    };

    // Agent starting position to each highlevel action
    enginetype::GridCell startCell = gridinfo::getPlayerPosition();
    for (auto const & option : availableOptions_) {
        uint64_t hash = tlshash::itemPairHash(availableOptions_, multiplier_, option, option);
        option->runAStar(startCell, option->getGoalCell());
        // Walk along base path and if a rock is above, set as restriction
        // addRestrictionsIfApplicable(hash, option->getSolutionPath());
        for (auto const & cell :option->getSolutionPath()) {
            if (std::find(rockCells.begin(), rockCells.end(), cell) != rockCells.end()) {
                restrictedCellsByOption_[hash].insert(gridinfo::cellToIndex(cell));
                ++restrictedCellsByOptionCount_[hash];
                PLOGE_(logger::FileLogger) << option->toString() << ", (" << cell.x << ", " << cell.y << ")";
            }
        }
    }
    PLOGE_(logger::FileLogger) << "----------------";
    // Pair options
    for (auto const & hash : tlshash::allItemsPairHashes(availableOptions_, multiplier_)) {
        std::array<std::size_t, 2> pair = tlshash::hashToItemIndexPair(hash, multiplier_);
        BaseOption* prev = availableOptions_[pair[0]];
        BaseOption* curr = availableOptions_[pair[1]];
        curr->runAStar(prev->getGoalCell(), curr->getGoalCell());

        if (pair[0] == 1 && pair[1] == 4) {
            PLOGE_(logger::FileLogger) << curr->getSolutionPath().size();
        }

        // Walk along base path and if a rock is above, set as restriction
        // addRestrictionsIfApplicable(hash, curr->getSolutionPath());
        for (auto const & cell :curr->getSolutionPath()) {
            if (std::find(rockCells.begin(), rockCells.end(), cell) != rockCells.end()) {
                restrictedCellsByOption_[hash].insert(gridinfo::cellToIndex(cell));
                ++restrictedCellsByOptionCount_[hash];
                PLOGE_(logger::FileLogger) << prev->toString() << " -> " << curr->toString() << ", (" << cell.x << ", " << cell.y << ")";
            }
        }
    }
#endif

    // Set initial levin node
    openLevinNodes_.clear(); 
    for (auto const & option : availableOptions_) {
        option->setAvoidNonGoalCollectibleCells(true);
    }

#ifndef SINGLE_PATH
    for (auto const & option : availableOptions_) {
        int numGem = elementproperty::getItemGemCount(gridinfo::getSpriteGridCell(option->getSpriteID()));
        bool hasDoor = elementproperty::isExit(gridinfo::getSpriteGridCell(option->getSpriteID()));
        uint64_t itemPairHash = tlshash::itemPairHash(availableOptions_, multiplier_, option, option);
    #ifndef MANUAL_CONSTRAINTS
        openLevinNodes_.insert({itemPairHash, 0, 0, 0, CombinatorialPartition(0), numGem, hasDoor});
    #else
        std::vector<BaseOption*> path = {option, option};
        setPathRestrictionSet(itemPairHash, path);
        int restriction_count = restrictedCellsByPath_[itemPairHash].size();
        openLevinNodes_.insert({itemPairHash, 0, 0, restriction_count, CombinatorialPartition(restriction_count), numGem, hasDoor});
    #endif
    }
#else
PLOGE_(logger::ConsoleLogger) << availableOptions_.size();
    // std::vector<BaseOption*> path{availableOptions_[6], availableOptions_[1], availableOptions_[2], availableOptions_[0], availableOptions_[7], 
    //     availableOptions_[4] , availableOptions_[3], availableOptions_[7], availableOptions_[8],
    //     availableOptions_[10], availableOptions_[8], availableOptions_[9], availableOptions_[5]
    // };
    std::vector<BaseOption*> path{availableOptions_[1], availableOptions_[6], availableOptions_[3], availableOptions_[2], availableOptions_[7], 
        availableOptions_[5], availableOptions_[7], availableOptions_[4]
    };
    uint64_t hash = tlshash::itemPathToHash(availableOptions_, path, multiplier_);
    // PLOGE_(logger::FileLogger) << hash << " " << path.size();
    setPathRestrictionSet(hash, path);
    int restriction_count = restrictedCellsByPath_[hash].size();
    // for (auto const & r : restrictedCellsByPath_[hash]) {
    //     PLOGE_(logger::FileLogger) << "restriction: (" << gridinfo::indexToCell(r).x << ", " << gridinfo::indexToCell(r).y << ")";
    // }
    openLevinNodes_.insert({hash, 0, 0, restriction_count, CombinatorialPartition(restriction_count), 6, true});
#endif
    // throw std::exception();
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
#ifndef MANUAL_CONSTRAINTS
    checkForMovedObjects();
#endif
}