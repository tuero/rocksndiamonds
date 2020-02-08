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



// CBS
void TwoLevelSearch::recursiveFindNextLevinHLP(std::vector<BaseOption*> &optionPath, int maxDepth) {
    // Full length path, calculate cost and store as hash
    if ((int)optionPath.size() == maxDepth) {
        if (optionPath[optionPath.size() - 1]->getOptionType() != OptionType::ToExit) {return;}
        uint64_t hash = optionPathToHash<std::vector<BaseOption*>>(optionPath);

        // We have exhausted all low level paths and we have not seen new constraints
        if (!newConstraintSeen(optionPath) && currentHighLevelPathComplete(hash)) {
            return;
        }

        NodeLevin node = {hash, getPathTimesVisited(optionPath), restrictionCountForPath(optionPath)};
        levinNodes_.push_back(node);
        return;
    }
    
    // Add each possible child and go into recursion
    for (auto const & option : availableOptions_) {
        auto iter = std::find(optionPath.begin(), optionPath.end(), option);
        if (iter != optionPath.end()) {continue;}
        optionPath.push_back(option);
        recursiveFindNextLevinHLP(optionPath, maxDepth);
        optionPath.pop_back();
    }
}

uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
std::mt19937 gen(seed);

void TwoLevelSearch::LevinTS() {
    levinNodes_.clear();
    std::vector<BaseOption*> optionPath;

    // Recursively construct paths and store costs
    PLOGD_(logger::FileLogger) << "Finding all path costs.";
    recursiveFindNextLevinHLP(optionPath, (int)availableOptions_.size());

    // Log paths
    logLevinNodes();

    // For each full path in map, add to distribution
    int minIndex = -1;
    double minCost = std::numeric_limits<double>::max();
    for (int i = 0; i < (int) levinNodes_.size(); i++) {
        if (levinNodes_[i].cost() < minCost) {
            minCost = levinNodes_[i].cost();
            minIndex = i;
        }
    }

    if (minIndex == -1) {
        PLOGE_(logger::FileLogger) << "No least Levin node found";
        PLOGE_(logger::ConsoleLogger) << "No least Levin node found";
    }

    // Choose min cost node
    NodeLevin chosenPathNode = levinNodes_[minIndex];
    PLOGD_(logger::FileLogger) << "Chosen high level path: " << chosenPathNode.hash;
    for (auto const & option : hashToOptionPath(chosenPathNode.hash)) {
        PLOGD_(logger::FileLogger) << option->toString();
    }

    // Set high level path
    highlevelPlannedPath_.clear();
    highlevelPlannedPath_ = hashToOptionPath(chosenPathNode.hash);
}


/**
 * Check and add new constraints found in previous searches, then
 * find the next high level path to try. Once the path is found, the
 * low level path algorithm is called.
 */
void TwoLevelSearch::highLevelSearch() {
    PLOGD_(logger::FileLogger) << "Starting high level search.";

    // Add new constraints found from previous attempt
    addNewConstraints();

    // High level search
    LevinTS(); 

    currentHighLevelPathHash = optionPathToHash<std::vector<BaseOption*>>(highlevelPlannedPath_);

    // Increment visited path node visits
    incrementPathTimesVisited<std::vector<BaseOption*>>(highlevelPlannedPath_);

    // Run middle level search on given path
    // Middle level means to find the next set of constraints
    lowLevelSearch();
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
            int index_a = enginehelper::cellToIndex(enginehelper::getSpriteGridCell(a->getSpriteID()));
            int index_b = enginehelper::cellToIndex(enginehelper::getSpriteGridCell(b->getSpriteID()));
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
