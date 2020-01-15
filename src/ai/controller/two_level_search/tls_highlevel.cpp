/**
 * @file: high_level_search.cpp
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
#include <set>
#include <algorithm>            // sort, find

// Includes
#include "base_option.h"
#include "option_types.h"
#include "engine_helper.h"
#include "logger.h"



// CBS
struct NodeCBS {
    std::unordered_map<int, std::vector<enginetype::GridCell>> constraints;
    int size = 0;
};

class CompareNodeCBS {
public:
    bool operator() (const NodeCBS &lhs, const NodeCBS &rhs) {
        return lhs.size > rhs.size;
        // return lhs.constraints.size() < rhs.constraints.size();
    }
};

typedef std::priority_queue<NodeCBS, std::vector<NodeCBS>, CompareNodeCBS> PriorityQueue;
std::unordered_map<uint64_t, PriorityQueue> openByPath;
std::unordered_map<uint64_t, std::vector<NodeCBS>> closedByPath;


/**
 * Runs one iteration of CBS on the currentHighLevelPathHash.
 * An iteration is counted as a single replay, which will use the restricted cells
 * set in the best node in OPEN, and will insert the children nodes into OPEN for
 * later iterations.
 */
void TwoLevelSearch::CBS() {
    // Safeguard against OPEN/CLOSED not seeing the current high level path before
    if (openByPath.find(currentHighLevelPathHash) == openByPath.end()) {
        openByPath[currentHighLevelPathHash] = {};
    }
    if (closedByPath.find(currentHighLevelPathHash) == closedByPath.end()) {
        closedByPath[currentHighLevelPathHash] = {};
    }

    // Get the OPEN/CLOSED for the given high level path
    PriorityQueue &open = openByPath[currentHighLevelPathHash];
    std::vector<NodeCBS> &closed = closedByPath[currentHighLevelPathHash];


    // Check every option in the planned path
    for (auto const & hash : givenPathOptionPairHashes(highlevelPlannedPath_)) {
        for (auto const & constraint : restrictedCellsByOption_[hash]) {
            bool isKnown = std::find(knownConstraints_[hash].begin(), knownConstraints_[hash].end(), constraint) != knownConstraints_[hash].end();
            if (isKnown) {continue;}

            knownConstraints_[hash].push_back(constraint);

            // If constraint is new, we need to consider adding to any node in closed
            // This would involve knowing where the agent is before attempting this option. Doable but involved.
            // Just assume we need to add for now.
            for (auto const & node : closed) {
                // Create child with parent constraints, and add the new constraint
                NodeCBS child = node;
                child.constraints[hash].push_back(constraint);
                child.size += 1;
                open.push(child);
            }
        }
    }


    // If first time running, we need to set initial node
    if (open.empty() && closed.empty()) {
        PLOGD_(logger::ConsoleLogger) << "Setting first node";
        std::unordered_map<int, std::vector<enginetype::GridCell>> constraints;
        for (auto const & hash : givenPathOptionPairHashes(highlevelPlannedPath_)) {
            constraints[hash] = {};
        }
        open.push((NodeCBS){constraints, 0});
    }

    // If all nodes exhausted, signal to HLS 
    if (open.empty()) {
        PLOGE_(logger::ConsoleLogger) << "ALL OPTIONS EMPTY";
    }

    // Constraints updated, now we do one iteration of search
    // Get best node in OPEN and add to CLOSED
    NodeCBS P = open.top();
    open.pop();
    closed.push_back(P);

    // Set constraints from P for each node in high level
    int index = 0;
    for (auto const & hash : givenPathOptionPairHashes(highlevelPlannedPath_)) {
        highlevelPlannedPath_[index++]->setRestrictedCells(P.constraints[hash]);
    }


    // The actual path validation will happen real time by controller. 
    // Here we just go ahead and add children to OPEN. If path is solved then we won't reenter.
    // Children inherit all constraints from parent, and add one single constraint.
    for (auto const & hash : givenPathOptionPairHashes(highlevelPlannedPath_)) {
        std::vector<enginetype::GridCell> &knownConstraitsForOption = knownConstraints_[hash];
        for (auto const & constraint : knownConstraitsForOption) {
            // If node currently doesn't have this constraint, create child for it
            if (std::find(P.constraints[hash].begin(), P.constraints[hash].end(), constraint) == P.constraints[hash].end()) {
                NodeCBS child = P;
                child.constraints[hash].push_back(constraint);
                child.size += 1;
                open.push(child);
            }
        }
    }
}


void TwoLevelSearch::LevinTS() {
    // Find each path and calculate path costs
    // Flip coin at each step
    highlevelPlannedPath_.clear();
}


/**
 * Check and add new constraints found in previous searches, then
 * find the next high level path to try. Once the path is found, the
 * low level path algorithm is called.
 */
void TwoLevelSearch::highLevelSearch() {
    // Add new constraints found from previous attempt
    addNewConstraints();

    // Create deterministic path
    highLevelSearchGemsInOrder();
    // smartAStar();

    currentHighLevelPathHash = optionPathToHash(highlevelPlannedPath_);

    // Run middle level search on given path
    // Middle level means to find the next set of constraints
    CBS();
}


/**
 * Find the path of high level options which corresponds to the collectible sprites
 * in order of (row, col), with the exit at the end. This is a deterministic path
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
