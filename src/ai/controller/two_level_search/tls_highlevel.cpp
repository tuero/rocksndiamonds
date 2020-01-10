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


void TwoLevelSearch::addNewConstraints() {
    bool flag = false;
    for (auto const & option : availableOptions_) {
        std::vector<enginetype::GridCell> &optionRestrictions = restrictedCellsByOption_[option];
        for (auto const & restriction : spritesMoved[option]) {
            // Add restriction if cell isn't already restricted
            bool cellRestricted = std::find(optionRestrictions.begin(), optionRestrictions.end(), restriction.cell) != optionRestrictions.end();
            if (!cellRestricted) {
                flag = true;
                optionRestrictions.push_back(restriction.cell);
            }
        }
    }
    if (flag) {
        int count = 0;
        for (auto const & option : availableOptions_) {
            count += restrictedCellsByOption_[option].size();
        }
        PLOGE_(logger::ConsoleLogger) << "New constraints added, total = " << count;
    }
}


// CBS
struct NodeCBS {
    std::unordered_map<BaseOption*, std::vector<enginetype::GridCell>> constraints;
    int size = 0;
};
// auto compareNodeCBS = [] (const NodeCBS &lhs, const NodeCBS &rhs) {return lhs.constraints.size() < rhs.constraints.size();};

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


void TwoLevelSearch::CBS() {
    if (openByPath.find(currentHighLevelPathHash) == openByPath.end()) {
        openByPath[currentHighLevelPathHash] = {};
    }
    if (closedByPath.find(currentHighLevelPathHash) == closedByPath.end()) {
        closedByPath[currentHighLevelPathHash] = {};
    }

    PriorityQueue &open = openByPath[currentHighLevelPathHash];
    std::vector<NodeCBS> &closed = closedByPath[currentHighLevelPathHash];


    // Check every option in the planned path
    for (auto const & option : highlevelPlannedPath_) {
        // Check if it exists in known constraints
        for (auto const & constraint : restrictedCellsByOption_[option]) {
            bool isKnown = std::find(knownConstraints_[option].begin(), knownConstraints_[option].end(), constraint) != knownConstraints_[option].end();
            if (isKnown) {continue;}

            knownConstraints_[option].push_back(constraint);

            // If constraint is new, we need to consider adding to any node in closed
            // This would involve knowing where the agent is before attempting this option. Doable but involved.
            // Just assume we need to add for now.
            for (auto const & node : closed) {
                // Create child with parent constraints, and add the new constraint
                NodeCBS child = node;
                child.constraints[option].push_back(constraint);
                child.size += 1;
                open.push(child);
            }
        }
    }

    // If first time running, we need to set initial node
    if (open.empty() && closed.empty()) {
        PLOGE_(logger::ConsoleLogger) << "Setting first node";
        std::unordered_map<BaseOption*, std::vector<enginetype::GridCell>> constraints;
        for (auto const & option : highlevelPlannedPath_) {
            constraints[option] = {};
        }
        open.push((NodeCBS){constraints, 0});
    }

    // If all nodes exhausted, signal to HLS 
    if (open.empty()) {
        PLOGE_(logger::ConsoleLogger) << "ALL OPTIONS EMPTY";
    }

    // Constraints updated, now we do one iteration of search
    // Get best node in OPEN and add to CLOSED

    // 1. Everytime we put a node into open, we hash and save
    // 1. Before putting into open, we check if hash exists
    // 2. Put a flag on new constraints and track
    // bool flag = false;
    // NodeCBS P;
    // PriorityQueue tempopen = open;
    // std::vector<NodeCBS> allNodes = closed;
    // int index = 0;
    // while(!tempopen.empty()) {
    //     allNodes.push_back(tempopen.top());
    //     tempopen.pop();
    // }
    // while (true) {
    //     index = 0;
    //     if (open.empty()) {
    //         PLOGE_(logger::ConsoleLogger) << "Shouldn't get here, ALL OPTIONS EMPTY";
    //     }
    //     P = open.top();
    //     open.pop();
    //     for (auto const & node : allNodes) {
    //         if (index - closed.size() == 0) {continue;}
    //         flag = true;
    //         for (auto const & option : highlevelPlannedPath_) {
    //             std::set<enginetype::GridCell> s1;
    //             std::set<enginetype::GridCell> s2;
    //             s1.insert(P.constraints[option].begin(), P.constraints[option].end());
    //             s2.insert(node.constraints.at(option).begin(), node.constraints.at(option).end());
    //             if (s1 != s2) {flag = false;}
    //         }
    //         index += 1;
    //         // Nodes have same constraints, skip
    //         if (flag) {break;}
    //     }
    //     if (!flag){break;}

    // }
    NodeCBS P = open.top();
    open.pop();
    closed.push_back(P);

    // Set constraints from P for each node in high level
    for (auto const & option : highlevelPlannedPath_) {
        option->setRestrictedCells(P.constraints[option]);
    }

    // The actual path validation will happen real time by controller. 
    // Here we just go ahead and add children to OPEN. If path is solved then we won't reenter.
    // Children inherit all constraints from parent, and add one single constraint.
    for (auto const & option : highlevelPlannedPath_) {
        std::vector<enginetype::GridCell> &knownConstraitsForOption = knownConstraints_[option];
        // Look over all known constraints for the option
        for (auto const & constraint : knownConstraitsForOption) {
            // If node currently doesn't have this constraint, create child for it
            if (std::find(P.constraints[option].begin(), P.constraints[option].end(), constraint) == P.constraints[option].end()) {
                NodeCBS child = P;
                child.constraints[option].push_back(constraint);
                child.size += 1;
                open.push(child);
            }
        }
    }
}


// Put in highlevel
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


void TwoLevelSearch::smartAStar() {
    HighLevelNode startNode = {nullptr, nullptr, nullptr, 0, (double)enginehelper::getLevelRemainingGemsNeeded()};

    // Search data structures
    std::unordered_map<BaseOption*, HighLevelNode> open;
    std::unordered_map<BaseOption*, HighLevelNode> closed;

    // Initialize with start node
    open[startNode.id] = startNode;

    while (!open.empty()) {
        // Pull next node and update data structures
        HighLevelNode node = {nullptr, nullptr, nullptr, std::numeric_limits<float>::lowest(), 0};
        for (auto it = open.begin(); it != open.end(); ++it) {
            if (it->second.g + it->second.h > node.g + node.h) {
                node = it->second;
            }
        }

        if (node.g == std::numeric_limits<float>::lowest()) {
            PLOGE_(logger::ConsoleLogger) << "Cannot find a node to pull";
            break;
        }

        open.erase(node.id);
        closed[node.id] = node;

        PLOGD_(logger::ConsoleLogger) << "Pulling option: " << (node.option == nullptr ? "Root" : node.option->toString());

        // Goal condition check
        // Goal is if we reach the exit
        if (node.option && node.option->getOptionType() == OptionType::ToExit) {
            PLOGD_(logger::ConsoleLogger) << "Found solution.";
            highlevelPlannedPath_.clear();
            while(node.id) {
                highlevelPlannedPath_.push_front(node.option);
                node = closed[node.parentId];
            }
            return;
        }

        // Expand children (each option)
        for (auto const & option : availableOptions_) {
            double dg = enginehelper::getLevelRemainingGemsNeeded();
            
            // if an option is not valid, then we skip
            // OptionToExit -> Must have enough gems collected in history
            if (option->getOptionType() == OptionType::ToExit) {
                HighLevelNode current = node;
                int gemsNeeded = enginehelper::getLevelRemainingGemsNeeded();
                while (current.id) {
                    if (current.option->getOptionType() == OptionType::ToCollectibleSprite) {
                        enginetype::GridCell cell = enginehelper::getSpriteGridCell(current.option->getSpriteID());
                        gemsNeeded -= enginehelper::getItemGemCount(cell);
                    }
                    current = closed[current.parentId];
                }
                // If not enough diamonds were collected in our path, then we cannot go to exit
                if (gemsNeeded > 0) {continue;}
            }
            // OptionCollectibleSprite -> Must have not previously collected the item
            else if (option->getOptionType() == OptionType::ToCollectibleSprite) {
                HighLevelNode current = node;
                int spriteId = option->getSpriteID();
                dg = enginehelper::getItemGemCount(enginehelper::getSpriteGridCell(spriteId));
                bool hasSeen = false;
                while (current.id) {
                    // We have collected this collectible previously
                    if (current.option->getOptionType() == OptionType::ToCollectibleSprite && current.option->getSpriteID() == spriteId) {
                        hasSeen = true;
                        break;
                    }
                    current = closed[current.parentId];
                }
                if (hasSeen) {continue;}
            }
            // Otherwise we don't know how to handle
            else {
                continue;
            }

            PLOGD_(logger::ConsoleLogger) << "Expanding option: " << option->toString();

            double newG = node.g + dg;
            double h = std::max(enginehelper::getLevelRemainingGemsNeeded() - newG, 0.0);

            // Node generated but not expanded
            if (open.find(option) != open.end()) {
                // Check if new path cheaper
                if (open[option].g > newG) {continue;}
                open.erase(option);
            }
            // Node already expanded
            else if (closed.find(option) != closed.end()) {
                // Check if new path cheaper
                if (closed[option].g > newG) {continue;}
                closed.erase(option);
            }
            open[option] = {option, node.id, option, newG, h};
        }
    }
}