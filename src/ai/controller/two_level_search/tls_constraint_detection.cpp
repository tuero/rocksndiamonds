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
#include "util/tls_hash.h"
#include "engine_types.h"
#include "engine_helper.h"
#include "logger.h"

using namespace enginehelper;



int TwoLevelSearch::restrictionCountForPath(const std::vector<BaseOption*> &path) {
    std::vector<uint64_t> pathHashes = tlshash::givenPathItemPairHashes(availableOptions_, multiplier_, path);
    auto lambda = [&](int sum, uint64_t hash) {return sum + restrictedCellsByOptionCount_[hash];};
    return std::accumulate(pathHashes.begin(), pathHashes.end(), 0, lambda);
}


/**
 * Check if falling object is due to player
 */
bool playerCausedObjectFall(const enginetype::GridCell &playerCell, const enginetype::GridCell &objectCell) {
    int dx = objectCell.x - playerCell.x;
    int dy = playerCell.y - objectCell.y;
    return (dx == 0 && dy == 0) || (dx == 0 && dy == 1);
}


/**
 * Update any nodes in LevinTS open that had a new constraint added on its path.
 */
void TwoLevelSearch::updateAffectedLevinNodes(uint64_t hash) {
    std::set<NodeLevin, CompareLevinNode> updatedNodes;
    for (auto it = openLevinNodes_.begin(); it != openLevinNodes_.end(); ) {
        std::vector<uint64_t> nodePathHashes = tlshash::pathHashToItemPairHash(it->hash, availableOptions_, multiplier_);

        // Levin node in open wasn't affected by new constraint
        if (std::find(nodePathHashes.begin(), nodePathHashes.end(), hash) == nodePathHashes.end()) {
            ++it;
            continue;
        } 
        else {
            auto lambda = [&](int sum, uint64_t hash) {return sum + restrictedCellsByOptionCount_[hash];};
            int totalConstraintCount = std::accumulate(nodePathHashes.begin(), nodePathHashes.end(), 0, lambda);

            // New node will be added back once we complete iteration
            NodeLevin node = *it;
            node.timesVisited = 0;
            node.numConstraints = totalConstraintCount;
            node.combinatorialPartition.reset(totalConstraintCount);
            updatedNodes.insert(node);
            openLevinNodes_.erase(it++);
        }
    }

    // Add back updated nodes
    openLevinNodes_.insert(updatedNodes.begin(), updatedNodes.end());
}


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
        // Wasn't just falling or falling but not by player, so skip.
        if (!gridinfo::getWasJustFalling(cell) || !playerCausedObjectFall(playerCells_[0], cell)) {continue;}

        uint64_t hash = tlshash::itemPairHash(availableOptions_, multiplier_, currentOption_, previousOption_);
        int index = gridinfo::cellToIndex(playerCells_[0]);

        // Restricted cell already seen
        if (restrictedCellsByOption_[hash].find(index) != restrictedCellsByOption_[hash].end()) {continue;}
            
        // Save newly seen restriction
        restrictedCellsByOption_[hash].insert(index);
        ++restrictedCellsByOptionCount_[hash];

        // Check if nodes in open need to be reset to account for new restrictions
        updateAffectedLevinNodes(hash);
    }
    
}
