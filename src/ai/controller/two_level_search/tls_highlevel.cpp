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
#include <algorithm>            // sort, find

// Includes
#include "util/tls_hash.h"
#include "base_option.h"
#include "option_types.h"
#include "engine_helper.h"
#include "logger.h"

using namespace enginehelper;


// Flag to skip level tries if path doesn't have minimum number
// of diamonds, or doesn't have the exit.
#define SKIP_REQUIRED_GEMS_DOOR


void TwoLevelSearch::setLowLevelConstraints(const NodeLevin &node) {
    highlevelPlannedPath_.clear();

#ifdef SKIP_REQUIRED_GEMS_DOOR
    if (node.numGems >= levelinfo::getLevelGemsNeeded() && node.hasDoor) {
#endif
        highlevelPlannedPath_ = tlshash::hashToItemPath(node.hash, multiplier_, availableOptions_);
        uint64_t constraintBits = tlsbits::getNextConstraintBits(node.combinatorialPartition);

        // Set constraints from bits for each option in the high level path
        uint64_t mask = 1;
        std::vector<uint64_t> pathHashes = tlshash::givenPathItemPairHashes(availableOptions_, multiplier_, highlevelPlannedPath_);
        for (int i = 0; i < (int)pathHashes.size(); i++) {
            highlevelPlannedPath_[i]->clearRestrictedCells();
            for (auto const & constraint : restrictedCellsByOption_[pathHashes[i]]) {
                // Bit indicates that this constraint should be set.
                if (constraintBits & (mask)) {
                    highlevelPlannedPath_[i]->addRestrictedCell(constraint);
                }
                mask = mask << 1;
            }
        }
        PLOGD_(logger::FileLogger) << "Node: " << node.hash << ", constraints: " << node.numConstraints 
            << " , visited: " << node.timesVisited;
#ifdef SKIP_REQUIRED_GEMS_DOOR
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
    // This is the node which will be simulated (after we add children to open)
    setLowLevelConstraints(node);

    // Add children (additional high-level action)
    if (node.timesVisited == 0) {
        std::vector<BaseOption*> nodeOptions = tlshash::hashToItemPath(node.hash, multiplier_, availableOptions_);
        std::vector<BaseOption*> childOptions = nodeOptions;
        childOptions.push_back(nullptr);

        // If option not already on path, add as a child
        for (auto const & option : availableOptions_) {
            if (std::find(nodeOptions.begin(), nodeOptions.end(), option) == nodeOptions.end()) {
                childOptions.back() = option;
                uint64_t hash = tlshash::itemPathToHash(availableOptions_, childOptions, multiplier_);
                int numGems = node.numGems + elementproperty::getItemGemCount(gridinfo::getSpriteGridCell(option->getSpriteID()));
                bool hasDoor = elementproperty::isExit(gridinfo::getSpriteGridCell(option->getSpriteID()));
                openLevinNodes_.insert({hash, 0, restrictionCountForPath(childOptions), CombinatorialPartition(restrictionCountForPath(childOptions)), numGems, hasDoor});
            }
        }
    }

    // If not exhausted, add child (same high-level path but increment bitwise index)
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
    highlevelPlannedPath_.clear();
    while (highlevelPlannedPath_.empty()) {
        modifiedLevinTS();
    }

    currentHighLevelPathHash_ = tlshash::itemPathToHash(availableOptions_, highlevelPlannedPath_, multiplier_);
    incrementPathTimesVisited();
}

