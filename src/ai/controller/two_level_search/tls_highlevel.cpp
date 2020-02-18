/**
 * @file: tel_highlevel.cpp
 *
 * @brief: High level search functionality for Masters thesis controller.
 * 
 * @author: Jake Tuero
 * Date: January 2020
 * Contact: tuero@ualberta.ca
 */


#include "two_level_search.h"

// Standard Libary/STL
#include <vector>
#include <queue>
#include <deque>
#include <set>
#include <unordered_set>
#include <algorithm>            // sort, find
#include <random>               // random_device, mt19937, discrete_distribution
#include <limits>               // numeric_limits

// Includes
#include "base_option.h"
#include "option_types.h"
#include "engine_helper.h"
#include "logger.h"

using namespace enginehelper;



#define SKIP_NOT_ENOUGH_GEMS


void TwoLevelSearch::setLowLevelConstraints(const NodeLevin &node) {
    highlevelPlannedPath_.clear();

#ifdef SKIP_NOT_ENOUGH_GEMS
    if (node.numGems >= levelinfo::getLevelGemsNeeded()) {
#endif
        highlevelPlannedPath_ = hashToOptionPath(node.hash);
        uint64_t constraintBits = node.combinatorialPartition.getNextConstraintBits();

        // Set constraints from bits for each option in the high level path
        uint64_t mask = 1;
        std::vector<uint64_t> pathHashes = givenPathOptionPairHashes(highlevelPlannedPath_);
        for (int i = 0; i < (int)pathHashes.size(); i++) {
            std::unordered_set<int> constraints;
            for (auto const & constraint : restrictedCellsByOption_[pathHashes[i]]) {
                if (constraintBits & (mask)) {
                    constraints.insert(constraint);
                }
                mask = mask << 1;
            }
            highlevelPlannedPath_[i]->setRestrictedCells(constraints);
        }
#ifdef SKIP_NOT_ENOUGH_GEMS
    }
#endif
}

void TwoLevelSearch::modifiedLevinTS() {
    // No available levin nodes.
    if (openLevinNodes_.empty()) {
        PLOGE_(logger::FileLogger) << "All levin nodes exhausted.";
        PLOGE_(logger::ConsoleLogger) << "All levin nodes exhausted.";
        throw std::exception();
    }

    NodeLevin node = *(openLevinNodes_.begin());
    openLevinNodes_.erase(openLevinNodes_.begin());

    // Set high level path
    setLowLevelConstraints(node);

    // Add children
    if (node.timesVisited == 0) {
        std::vector<BaseOption*> nodeOptions = hashToOptionPath(node.hash);
        std::vector<BaseOption*> childOptions = nodeOptions;
        childOptions.push_back(nullptr);

        for (auto const & option : availableOptions_) {
            if (std::find(nodeOptions.begin(), nodeOptions.end(), option) == nodeOptions.end()) {
                int numGems = node.numGems + elementproperty::getItemGemCount(gridinfo::getSpriteGridCell(option->getSpriteID()));
                childOptions.back() = option;
                uint64_t hash = optionPathToHash(childOptions);
                openLevinNodes_.insert({hash, 0, restrictionCountForPath(childOptions), CombinatorialPartition(), numGems});
            }
        }
    }

    // If not exhausted, add child
    if (!node.combinatorialPartition.isComplete()) {
        ++node.timesVisited;
        openLevinNodes_.insert(node);
    }
}


/**
 * Check and add new constraints found in previous searches, then
 * find the next high level path to try. Once the path is found, the
 * low level path algorithm is called.
 */
void TwoLevelSearch::highLevelSearch() {
    PLOGD_(logger::FileLogger) << "Starting high level search.";

    // High level search
    // LevinTS(); 
    highlevelPlannedPath_.clear();
    while (highlevelPlannedPath_.empty()) {
        modifiedLevinTS();
    }
    // modifiedLevinTS();

    currentHighLevelPathHash = optionPathToHash<std::vector<BaseOption*>>(highlevelPlannedPath_);
    incrementPathTimesVisited<std::vector<BaseOption*>>(highlevelPlannedPath_);

    #if 0
    // Increment visited path node visits
    incrementPathTimesVisited<std::vector<BaseOption*>>(highlevelPlannedPath_);

    // Run middle level search on given path
    // Middle level means to find the next set of constraints
    lowLevelSearch();
    #endif
}


/**
 * Find the path of high level options which corresponds to the collectible sprites in order of (row, col), with the exit at the end. This is a deterministic path
 * that never changes, good for testing sanity.
 */
void TwoLevelSearch::highLevelSearchGemsInOrder() {
    // Clear current solution
    highlevelPlannedPath_.clear();

    // Sort options by gem index first, then exit.
    std::sort(availableOptions_.begin(), availableOptions_.end(), 
        [](BaseOption* a, BaseOption* b) -> bool 
        {
            bool isexit_a = a->getOptionType() == OptionType::ToExit;
            bool isexit_b = b->getOptionType() == OptionType::ToExit;
            int index_a = gridinfo::cellToIndex(gridinfo::getSpriteGridCell(a->getSpriteID()));
            int index_b = gridinfo::cellToIndex(gridinfo::getSpriteGridCell(b->getSpriteID()));
            return index_a < index_b && !(isexit_a && !isexit_b);
        });
    
    // Path is to visit each gem in order, then exit
    for (auto const & option : availableOptions_) {
        highlevelPlannedPath_.push_back(option);
    }
}


void TwoLevelSearch::highLevelSearchDeterministic() {
    highlevelPlannedPath_.clear();
    std::vector<int> optionSpriteOrder = {24, 22, 7, 15, 26};

    for (auto const & spriteID : optionSpriteOrder) {
        for (auto const & option : availableOptions_) {
            if (option->getSpriteID() == spriteID) {
                highlevelPlannedPath_.push_back(option);
                break;
            }
        }
    }
}
