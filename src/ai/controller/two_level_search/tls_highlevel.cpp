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
#include "base_option.h"
#include "option_types.h"
#include "engine_helper.h"
#include "logger.h"

#include "statistics.h"

using namespace enginehelper;


// Flag to skip level tries if path doesn't have minimum number
// of diamonds, or doesn't have the exit.
#define SKIP_REQUIRED_GEMS_DOOR

#ifndef SET_RESTRICTIONS
void TwoLevelSearch::setLowLevelConstraints(const NodeLevin &node) {
    highlevelPlannedPath_.clear();

#ifdef SKIP_REQUIRED_GEMS_DOOR
    if (node.numGems >= levelinfo::getLevelGemsNeeded() && node.hasDoor) {
#endif
        highlevelPlannedPath_ = node.path;
        uint64_t constraintBits = tlsbits::getNextConstraintBits(node.combinatorialPartition);

        // Set constraints from bits for each option in the high level path
        uint64_t mask = 1;
        std::vector<uint64_t> pathHashes = tlshash::pathToPairHashes(availableOptions_, highlevelPlannedPath_);
        for (int i = 0; i < (int)pathHashes.size(); i++) {
            highlevelPlannedPath_[i]->clearRestrictedCells();
            for (auto const & constraint : restrictedCellsByOption_[pathHashes[i]]) {
            // for (auto & constraint : node.r_for_path.at(pathHashes[i])) {
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
#else
void TwoLevelSearch::setLowLevelConstraints(const NodeLevin &node) {
    highlevelPlannedPath_.clear();

#ifdef SKIP_REQUIRED_GEMS_DOOR
    if (node.numGems >= levelinfo::getLevelGemsNeeded() && node.hasDoor) {
#endif
        highlevelPlannedPath_ = node.path;
        if (node.combinatorialPartition.isComplete()) {
            PLOGE_(logger::FileLogger) << "All constraints checked.";
            throw std::exception();
        }
        uint64_t constraintBits = tlsbits::getNextConstraintBits(node.combinatorialPartition);
        // PLOGD_(logger::FileLogger) << std::bitset<12>(constraintBits);

        // Set constraints from bits for each option in the high level path
        uint64_t mask = 1;
        std::unordered_set<int> restrictions;
        for (auto & constraint : restrictedCellsByPath_[node.hash]) {
            if (constraintBits & (mask)) {
                restrictions.insert(constraint);
            }
            mask = mask << 1;
        }

        for (auto & option : highlevelPlannedPath_) {
            option->setRestrictedCells(restrictions);
        }
        
        // Iterate restriction set and set if option pair in path has the restriction as a possible option
        // std::vector<uint64_t> pathHashes = tlshash::givenPathItemPairHashes(availableOptions_, multiplier_, highlevelPlannedPath_);
        // for (int i = 0; i < (int)pathHashes.size(); i++) {
        //     highlevelPlannedPath_[i]->clearRestrictedCells();
        //     for (auto const & constraint : restrictedCellsByOption_[pathHashes[i]]) {
        //         if (restrictions.find(constraint) != restrictions.end()) {
        //             highlevelPlannedPath_[i]->addRestrictedCell(constraint);
        //             if (std::bitset<12>(constraintBits) == std::bitset<12>("110100000011")) {
        //                 PLOGE_(logger::FileLogger) << highlevelPlannedPath_[i]->toString() << ", cellx=" << gridinfo::indexToCell(constraint).x << 
        //                 " y=" << gridinfo::indexToCell(constraint).y;
        //             }
        //         }
        //     }
        // }
        // PLOGD_(logger::FileLogger) << "Node: " << node.hash << ", constraints: " << node.numConstraints 
        //     << " , visited: " << node.timesVisited;
#ifdef SKIP_REQUIRED_GEMS_DOOR
    }
#endif
}
#endif

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
        std::vector<BaseOption*> nodePath = node.path;
        std::vector<BaseOption*> childPath = nodePath;
        childPath.push_back(nullptr);

        // If option not already on path, add as a child
        for (auto const & option : availableOptions_) {
            bool in_path = std::find(nodePath.begin(), nodePath.end(), option) != nodePath.end();
            bool valid_door = option->getOptionType() == OptionType::ToDoor && nodePath[nodePath.size()-1]->getOptionType() != OptionType::ToDoor;
            if (valid_door || !in_path) {
                childPath.back() = option;
                uint64_t nodeHash = tlshash::hashPath(availableOptions_, childPath);
                int numGems = node.numGems + elementproperty::getItemGemCount(gridinfo::getSpriteGridCell(option->getSpriteID()));
                bool hasDoor = elementproperty::isExit(gridinfo::getSpriteGridCell(option->getSpriteID()));
            #ifndef SET_RESTRICTIONS
                int restriction_count = restrictionCountForPath(childPath);
                openLevinNodes_.insert({nodeHash, childPath.size(), 0, restriction_count, CombinatorialPartition(restriction_count), numGems, hasDoor});
            #else
                setPathRestrictionSet(nodeHash, childPath);
                int restriction_count = restrictedCellsByPath_[nodeHash].size();
                openLevinNodes_.insert({childPath, nodeHash, childPath.size(), 0, restriction_count, CombinatorialPartition(restriction_count), numGems, hasDoor});
            #endif
            }
        }
    }

    // If not exhausted, add child (same high-level path but increment bitwise index)
    if (!node.combinatorialPartition.isComplete()) {
        ++node.timesVisited;
        openLevinNodes_.insert(node);
    }
}


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
    // PLOGD_(logger::FileLogger) << "Starting high level search.";

    // High level search
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

