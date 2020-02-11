/**
 * @file: tls_lowlevel.cpp
 *
 * @brief: Low level search functionality for Masters thesis controller.
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
#include <numeric>              // accumulate

// Includes
#include "base_option.h"
#include "option_types.h"
#include "engine_helper.h"
#include "logger.h"

using namespace enginehelper;


void TwoLevelSearch::lowLevelSearch() {
    if (lowLevelSearchType == LowLevelSearchType::cbs) {
        CBS();
    }
    else if (lowLevelSearchType == LowLevelSearchType::combinatorial) {
        iterativeCombinatorial();
    }
}


bool TwoLevelSearch::currentHighLevelPathComplete(uint64_t hash) {
    if (lowLevelSearchType == LowLevelSearchType::cbs) {
        if (openByPath.find(hash) == openByPath.end()) {return false;}
        return openByPath[hash].empty() && !closedByPath[hash].empty();
    }
    else if (lowLevelSearchType == LowLevelSearchType::combinatorial) {
        if (combinatorialByPath.find(hash) == combinatorialByPath.end()) {return false;}
        return combinatorialByPath[hash].isComplete();
    }
    return false;
}


void TwoLevelSearch::iterativeCombinatorial() {
    // Safeguard against first time access
    if (combinatorialByPath.find(currentHighLevelPathHash) == combinatorialByPath.end()) {
        combinatorialByPath[currentHighLevelPathHash] = CombinatorialPartition();
    }

    CombinatorialPartition &combinatorialPartition = combinatorialByPath[currentHighLevelPathHash];

    // Get current path number of constraints known
    std::vector<int> numConstraintsOptionPair;
    std::vector<uint64_t> pathHashes = givenPathOptionPairHashes<std::vector<BaseOption*>>(highlevelPlannedPath_);
    for (auto const & hash : pathHashes) {
        numConstraintsOptionPair.push_back(restrictedCellsByOption_[hash].size());
    }

    // If new constraints detected, reset
    if (combinatorialPartition.requiresReset(numConstraintsOptionPair)) {
        combinatorialPartition.reset(numConstraintsOptionPair);
    }

    if (combinatorialPartition.isComplete()) {
        PLOGE_(logger::FileLogger) << "All constraints have been exhausted for the given high level path:" << currentHighLevelPathHash;
        PLOGE_(logger::ConsoleLogger) << "All constraints have been exhausted:" << currentHighLevelPathHash;
        return;
    }

    // Get next bit set
    std::vector<uint64_t> constraintBits = combinatorialPartition.getNextConstraintBits();

    // Set constraints from bits for each option in the high level path
    for (int i = 0; i < (int)pathHashes.size(); i++) {
        std::unordered_set<int> constraints;
        uint64_t mask = 0;
        for (auto const & constraint : restrictedCellsByOption_[pathHashes[i]]) {
            if (constraintBits[i] & (1 << mask++)) {
                constraints.insert(constraint);
            }
        }
        highlevelPlannedPath_[i]->setRestrictedCells(constraints);
    }
}


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
    for (auto const & hash : givenPathOptionPairHashes<std::vector<BaseOption*>>(highlevelPlannedPath_)) {
        for (auto const & constraint : restrictedCellsByOption_[hash]) {
            if (knownConstraints_[hash].find(constraint) != knownConstraints_[hash].end()) {continue;}
            knownConstraints_[hash].insert(constraint);

            // If constraint is new, we need to consider adding to any node in closed
            // This would involve knowing where the agent is before attempting this option. Doable but involved.
            // Just assume we need to add for now.
            for (auto const & node : closed) {
                // Create child with parent constraints, and add the new constraint
                NodeCBS child = node;
                child.constraints[hash].insert(constraint);
                child.size += 1;
                open.push(child);
                PLOGD_(logger::FileLogger) << "Updating to Open hash = " << hash << ", constraint = " << constraint;
                PLOGD_(logger::ConsoleLogger) << "Updating to Open hash = " << hash << ", constraint = " << constraint;
                for (auto const & tempHash : givenPathOptionPairHashes<std::vector<BaseOption*>>(highlevelPlannedPath_)) {
                    for (auto const & constraint : child.constraints[tempHash]) {
                        PLOGD_(logger::FileLogger) << "  hash: " << tempHash << ", constraint: " << constraint;
                        PLOGD_(logger::ConsoleLogger) << "  hash: " << tempHash << ", constraint: " << constraint;
                    }
                }
            }
        }
    }


    // If first time running, we need to set initial node
    if (open.empty() && closed.empty()) {
        PLOGD_(logger::ConsoleLogger) << "Setting first node";
        std::unordered_map<uint64_t, std::unordered_set<int>> constraints;
        for (auto const & hash : givenPathOptionPairHashes<std::vector<BaseOption*>>(highlevelPlannedPath_)) {
            constraints[hash] = {};
        }
        open.push((NodeCBS){constraints, 0});
    }

    // If all nodes exhausted, signal to HLS 
    if (open.empty()) {
        PLOGE_(logger::FileLogger) << "ALL OPTIONS EMPTY";
        PLOGE_(logger::ConsoleLogger) << "ALL OPTIONS EMPTY";
        return;
    }

    // Constraints updated, now we do one iteration of search
    // Get best node in OPEN and add to CLOSED
    NodeCBS P = open.top();
    open.pop();
    closed.push_back(P);
    PLOGD_(logger::FileLogger) << "Pulling from open = ";
    PLOGD_(logger::ConsoleLogger) << "Pulling from open = ";
    for (auto const & hash : givenPathOptionPairHashes<std::vector<BaseOption*>>(highlevelPlannedPath_)) {
        for (auto const & constraint : P.constraints[hash]) {
            PLOGD_(logger::FileLogger) << "  hash: " << hash << ", constraint: " << constraint;
            PLOGD_(logger::ConsoleLogger) << "  hash: " << hash << ", constraint: " << constraint;
        }
    }   

    // Set constraints from P for each node in high level
    int index = 0;
    PLOGD_(logger::ConsoleLogger) << "Setting restrictions:";
    PLOGD_(logger::FileLogger) << "Setting restrictions:";
    for (auto const & hash : givenPathOptionPairHashes<std::vector<BaseOption*>>(highlevelPlannedPath_)) {
        highlevelPlannedPath_[index]->setRestrictedCells(P.constraints[hash]);
        PLOGD_(logger::FileLogger) << highlevelPlannedPath_[index]->toString();
        PLOGD_(logger::ConsoleLogger) << highlevelPlannedPath_[index]->toString();
        for (auto const & constraint : P.constraints[hash]) {
            PLOGD_(logger::FileLogger) << "  hash: " << hash << ", constraint: " << constraint;
            PLOGD_(logger::ConsoleLogger) << "  hash: " << hash << ", constraint: " << constraint;
        }
        ++index;
    }


    // The actual path validation will happen real time by controller. 
    // Here we just go ahead and add children to OPEN. If path is solved then we won't reenter.
    // Children inherit all constraints from parent, and add one single constraint.
    for (auto const & hash : givenPathOptionPairHashes<std::vector<BaseOption*>>(highlevelPlannedPath_)) {
        std::unordered_set<int> &knownConstraitsForOption = knownConstraints_[hash];
        for (auto const & constraint : knownConstraitsForOption) {
            // If node currently doesn't have this constraint, create child for it
            if (P.constraints[hash].find(constraint) == P.constraints[hash].end()) {
                NodeCBS child = P;
                child.constraints[hash].insert(constraint);
                child.size += 1;
                open.push(child);
                PLOGD_(logger::FileLogger) << "Adding to Open hash = ";
                PLOGD_(logger::ConsoleLogger) << "Adding to Open hash = " ;
                for (auto const & tempHash : givenPathOptionPairHashes<std::vector<BaseOption*>>(highlevelPlannedPath_)) {
                    for (auto const & constraint : child.constraints[tempHash]) {
                        PLOGD_(logger::FileLogger) << "  hash: " << tempHash << ", constraint: " << constraint;
                        PLOGD_(logger::ConsoleLogger) << "  hash: " << tempHash << ", constraint: " << constraint;
                    }
                }
            }

        }
    }
}