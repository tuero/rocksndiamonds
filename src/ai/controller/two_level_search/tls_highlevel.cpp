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
#include <bitset>               // -- remove

// Includes
#include "util/tls_hash.h"
#include "util/tls_feature.h"
#include "base_option.h"
#include "option_types.h"
#include "engine_helper.h"
#include "logger.h"

#include "statistics.h"

using namespace enginehelper;


// Flag to skip level tries if path doesn't have minimum number
// of diamonds, or doesn't have the exit.
#define SKIP_REQUIRED_GEMS_DOOR


/**
 * Sets the constraints for the given bitpattern
 */
void TwoLevelSearch::setConstraintsFromBitpattern(const NodeLevin &node) {
    uint64_t mask = 1;
    uint64_t constraintBits = tlsbits::getNextConstraintBits(node.combinatorialPartition);
#ifndef SET_RESTRICTIONS
    std::vector<uint64_t> pathHashes = tlshash::pathToPairHashes(availableOptions_, highlevelPlannedPath_);
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
#else
    std::unordered_set<int> restrictions;
    for (auto & constraint : restrictedCellsByPath_[node.hash]) {
        // Bit indicates that this constraint should be set.
        if (constraintBits & (mask)) {
            restrictions.insert(constraint);
        }
        mask = mask << 1;
    }

    // Set constraints from bits for each option in the high level path
    for (auto & option : highlevelPlannedPath_) {
        option->setRestrictedCells(restrictions);
    }
#endif
}



/**
 * Set the constraints for each node on the high-level path, before we 
 * initiaze the low-level search
 */
void TwoLevelSearch::setLowLevelConstraints(NodeLevin &node) {
    highlevelPlannedPath_.clear();
#ifdef SKIP_REQUIRED_GEMS_DOOR
    if (node.numGems >= levelinfo::getLevelGemsNeeded() && node.hasDoor) {
#endif
        // Set high level path and ensure we have not run out of constraint combination
        // modifiedLevinTS() shouldn't insert completed nodes, but best to be safe
        highlevelPlannedPath_ = node.path;
        if (node.combinatorialPartition.isComplete()) {
            PLOGE_(logger::FileLogger) << "All constraints checked.";
            throw std::exception();
        }

        // Get constraints matching bitpattern and set constraints
        setConstraintsFromBitpattern(node);     

#ifdef SKIP_REQUIRED_GEMS_DOOR
    }
    else {
        node.wasSkipped = true;
    }
#endif
}


/**
 * Modified leveinTS.
 */
void TwoLevelSearch::modifiedLevinTS() {
    // No available levin nodes.
    if (openLevinNodes_.empty()) {
        PLOGE_(logger::FileLogger) << "All levin nodes exhausted.";
        PLOGE_(logger::ConsoleLogger) << "All levin nodes exhausted.";
        throw std::exception();
    }

    // Pull node
    NodeLevin node = *(openLevinNodes_.begin());
    openLevinNodes_.erase(openLevinNodes_.begin());
    node.timesVisited += 1;

    // Set high level path
    // This is the node which will be simulated (after we add children to open)
    setLowLevelConstraints(node);

    // Add children (additional high-level action)
    if (!node.hasExpanded) {
        node.hasExpanded = true;
        std::vector<BaseOption*> nodePath = node.path;
        std::vector<BaseOption*> childPath = nodePath;
        childPath.push_back(nullptr);

        // Try to expand (add child options)
        for (auto const & option : availableOptions_) {
            // We never want to revisit the same option unless its a door which isn't the immediate previous option as well (to prevent door -> door -> door etc.)
            bool in_path = std::find(nodePath.begin(), nodePath.end(), option) != nodePath.end();
            bool valid_door = option->getOptionType() == OptionType::ToDoor && nodePath[nodePath.size()-1]->getOptionType() != OptionType::ToDoor;

            // child option is either already in path, or its a door which is not valid, then continue
            if (!valid_door && in_path) {continue;}

            // Add option to children
            childPath.back() = option;

            // Get child node properties for constructor
            uint64_t childHash = tlshash::hashPath(availableOptions_, childPath);
            int numGems = node.numGems + elementproperty::getItemGemCount(gridinfo::getSpriteGridCell(option->getSpriteID()));
            bool hasDoor = elementproperty::isExit(gridinfo::getSpriteGridCell(option->getSpriteID()));
            int restriction_count = restrictionCountForPath(childPath);

            // Add child node to open
            openLevinNodes_.insert({childPath, tlsfeature::getNodePath(childPath), childHash, childPath.size(), 0, 
                restriction_count, CombinatorialPartition(restriction_count), numGems, hasDoor, false, false
            });
        }
    }

    // If not exhausted, add child (same high-level path but increment bitwise index)
    if (!node.combinatorialPartition.isComplete() && !node.wasSkipped) {
        // ++node.timesVisited;
        openLevinNodes_.insert(node);
    }
    // Otherwise, node exhausted, and so we add to close (as we've observed this runtime)
    else {
        closedLevinNodes_.insert(node);
    }
}


/**
 * For testing
 * Tree is a single node which gets pulled than readded, but increments times visited/constraint
 * bitset pattern.
 */
void TwoLevelSearch::singlePath() {
    NodeLevin node = *(openLevinNodes_.begin());
    openLevinNodes_.erase(openLevinNodes_.begin());

    setLowLevelConstraints(node);

    ++node.timesVisited;
    openLevinNodes_.insert(node);
    statistics::solutionConstraintCount[levelinfo::getLevelNumber()] = node.numConstraints;
}


/**
 * Check and add new constraints found in previous searches, then
 * find the next high level path to try. Once the path is found, the
 * low level path algorithm is called.
 */
void TwoLevelSearch::highLevelSearch() {
    highlevelPlannedPath_.clear();

#ifndef SINGLE_PATH
    while (highlevelPlannedPath_.empty()) {
        modifiedLevinTS();
    }
#else
    singlePath();
#endif

    currentHighLevelPathHash_ = tlshash::hashPath(availableOptions_, highlevelPlannedPath_);
    // incrementPathTimesVisited();
}

