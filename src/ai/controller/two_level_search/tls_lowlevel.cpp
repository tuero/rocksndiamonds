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

// Includes
#include "base_option.h"
#include "option_types.h"
#include "engine_helper.h"
#include "logger.h"


void TwoLevelSearch::lowLevelSearch() {
    CBS();
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
    if (consideredNodesByPath.find(currentHighLevelPathHash) == consideredNodesByPath.end()) {
        consideredNodesByPath[currentHighLevelPathHash] = {};
    }

    // Get the OPEN/CLOSED for the given high level path
    PriorityQueue &open = openByPath[currentHighLevelPathHash];
    std::vector<NodeCBS> &closed = closedByPath[currentHighLevelPathHash];
    std::vector<NodeCBS> &consideredNodes = consideredNodesByPath[currentHighLevelPathHash];


    // Check every option in the planned path
    // if (newConstraintFoundFlag_) {
        for (auto const & hash : givenPathOptionPairHashes<std::vector<BaseOption*>>(highlevelPlannedPath_)) {
            for (auto const & constraint : restrictedCellsByOption_[hash]) {
                // bool isKnown = std::find(knownConstraints_[hash].begin(), knownConstraints_[hash].end(), constraint) != knownConstraints_[hash].end();
                // if (isKnown) {continue;}

                // knownConstraints_[hash].push_back(constraint);
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
        newConstraintFoundFlag_ = false;
    // }


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
        // std::vector<enginetype::GridCell> &knownConstraitsForOption = knownConstraints_[hash];
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

            // if (std::find(P.constraints[hash].begin(), P.constraints[hash].end(), constraint) == P.constraints[hash].end()) {
            //     NodeCBS child = P;
            //     child.constraints[hash].push_back(constraint);
            //     child.size += 1;

            //     // Make sure we have not already touched a node with the same constraints
            //     // bool flag = false;
            //     // for (auto const & consideredNode : consideredNodes) {
            //     //     if (consideredNode.constraints == child.constraints) {
            //     //         PLOGE_(logger::FileLogger) << "Skipping duplicate node";
            //     //         PLOGE_(logger::ConsoleLogger) << "Skipping duplicate node";
            //     //         flag = true;
            //     //         break;
            //     //     }
            //     // }
            //     // if (!flag) {
            //     //     open.push(child);
            //     //     consideredNodes.push_back(child);
            //     // }
            //     open.push(child);
            // }
        }
    }
}