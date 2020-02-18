/**
 * @file: tls_constraint_detection.cpp
 *
 * @brief: Constraint detection for Masters thesis controller.
 * 
 * @author: Jake Tuero
 * Date: January 2020
 * Contact: tuero@ualberta.ca
 */


#include "two_level_search.h"

// Standard Libary/STL
#include <cmath>
#include <numeric>

// Includes
#include "engine_types.h"
#include "engine_helper.h"
#include "logger.h"

using namespace enginehelper;


bool TwoLevelSearch::newConstraintSeen(std::vector<BaseOption*> &optionPath) {
    // Haven't seen this one before
    if (combinatorialByPath.find(optionPathToHash<std::vector<BaseOption*>>(optionPath)) == combinatorialByPath.end()) {
        return true;
    }

    auto lambda = [&](int sum, uint64_t hash) {return sum + restrictedCellsByOptionCount_[hash];};
    std::vector<uint64_t> pathHashes = givenPathOptionPairHashes<std::vector<BaseOption*>>(optionPath);
    int totalConstraintCount = std::accumulate(pathHashes.begin(), pathHashes.end(), 0, lambda);
    CombinatorialPartition &combinatorialPartition = combinatorialByPath[optionPathToHash<std::vector<BaseOption*>>(optionPath)];
    return combinatorialPartition.requiresReset(totalConstraintCount);
}


template<typename T>
int TwoLevelSearch::restrictionCountForPath(const T &pathContainer) {
    std::vector<uint64_t> pathHashes = givenPathOptionPairHashes(pathContainer);
    auto lambda = [&](int sum, uint64_t hash) {return sum + restrictedCellsByOptionCount_[hash];};
    return std::accumulate(pathHashes.begin(), pathHashes.end(), 0, lambda);
}
template int TwoLevelSearch::restrictionCountForPath<std::vector<BaseOption*>> (const std::vector<BaseOption*>&);
template int TwoLevelSearch::restrictionCountForPath<std::deque<BaseOption*>> (const std::deque<BaseOption*>&);


/**
 * Check for newely moved objects as a result of player actions.
 * Objects are stored as sprite and gridcell pairs for a given pair of options (option from -> option to)
 * In addNewConstraints(), we store only the intersection of gridcells in restrictedCellsByOption_
 */
void TwoLevelSearch::checkForMovedObjects() {
    if (gridinfo::getPlayerPosition() != playerCells_[1]) {
        playerCells_[0] = playerCells_[1];
        playerCells_[1] = gridinfo::getPlayerPosition();
    }

    if (playerCells_[0] == (enginetype::GridCell){-1, -1}) {return;}
    
    for (auto const & cell : gridinfo::getMapSprites()) {
        // Wasn't just falling, so skip.
        if (!gridinfo::getWasJustFalling(cell)) {continue;}

        // Player caused falling sprite
        int dx = cell.x - playerCells_[0].x;
        int dy = playerCells_[0].y - cell.y;
        if (!(dx == 0 && dy == 0) && !(dx == 0 && dy == 1)) {continue;}

        uint64_t hash = optionPairHash(currentOption_, previousOption_);
        int index = gridinfo::cellToIndex(playerCells_[0]);

        // Restricted cell already seen
        if (restrictedCellsByOption_[hash].find(index) != restrictedCellsByOption_[hash].end()) {continue;}
            
        restrictedCellsByOption_[hash].insert(index);
        ++restrictedCellsByOptionCount_[hash];

        // Check if nodes in open need to be reset to account for new restrictions
        std::set<NodeLevin, CompareLevinNode> updatedNodes;
        for (auto it = openLevinNodes_.begin(); it != openLevinNodes_.end(); ) {
            std::vector<uint64_t> nodePathHashes = pathHashToOptionPairHash(it->hash);

            // Levin node in open wasn't affected by new constraint
            if (std::find(nodePathHashes.begin(), nodePathHashes.end(), hash) == nodePathHashes.end()) {
                ++it;
                continue;
            }

            int totalConstraintCount = 0;
            for (auto const & hash : nodePathHashes) {
                totalConstraintCount += restrictedCellsByOptionCount_[hash];
            }

            // New node will be added back once we complete iteration
            NodeLevin node = *it;
            node.timesVisited = 0;
            node.numConstraints = totalConstraintCount;
            node.combinatorialPartition.reset(totalConstraintCount);
            updatedNodes.insert(node);
            openLevinNodes_.erase(it++);
        }

        // Add back updated nodes
        openLevinNodes_.insert(updatedNodes.begin(), updatedNodes.end());
    }
    
}
