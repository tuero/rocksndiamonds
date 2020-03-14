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

void TwoLevelSearch::setPathRestrictionSet(uint64_t hash, const std::vector<BaseOption*> &path) {
    restrictedCellsByPath_[hash].clear();
    PLOGD_(logger::FileLogger) << "setting path restrictions";

    for (auto const & pairHash : tlshash::pathToPairHashes(availableOptions_, path)) {
        for (auto const & r : restrictedCellsByOption_[pairHash]) {
            if (restrictedCellsByPath_[hash].find(r) == restrictedCellsByPath_[hash].end()) {
                restrictedCellsByPath_[hash].insert(r);
                // PLOGD_(logger::FileLogger) << "x=" << gridinfo::indexToCell(r).x << ", y=" << gridinfo::indexToCell(r).y;
            }
        }
    }
    for (auto const & r : restrictedCellsByPath_[hash]) {
        PLOGD_(logger::FileLogger) << "x=" << gridinfo::indexToCell(r).x << ", y=" << gridinfo::indexToCell(r).y;
    }
}

int TwoLevelSearch::restrictionCountForPath(const std::vector<BaseOption*> &path) {
    std::vector<uint64_t> pathHashes = tlshash::pathToPairHashes(availableOptions_, path);
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
void TwoLevelSearch::updateAffectedLevinNodes(uint64_t optionPairHash) {
    std::set<NodeLevin, CompareLevinNode> updatedNodes;

    // Walk over levin nodes and check if nodes path contains affected option pair
    for (auto it = openLevinNodes_.begin(); it != openLevinNodes_.end(); ) {
        std::vector<uint64_t> nodePathHashes = tlshash::pathToPairHashes(availableOptions_, it->path);

        // Levin node in open wasn't affected by new constraint
        if (std::find(nodePathHashes.begin(), nodePathHashes.end(), optionPairHash) == nodePathHashes.end()) {
            ++it;
            continue;
        } 
        else {
        #ifndef SET_RESTRICTIONS
            auto lambda = [&](int sum, uint64_t hash) {return sum + restrictedCellsByOptionCount_[hash];};
            int totalConstraintCount = std::accumulate(nodePathHashes.begin(), nodePathHashes.end(), 0, lambda);
        #else
            setPathRestrictionSet(it->hash, it->path);
            int totalConstraintCount = restrictedCellsByPath_[it->hash].size();
            // PLOGD_(logger::ConsoleLogger) << "numconstraints " << totalConstraintCount;
        #endif

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
        if (initialState.Feld_[playerCells_[0].x][playerCells_[0].y] != enginetype::ELEMENT_SAND) {continue;}
        if (!gridinfo::getWasJustFalling(cell) || !playerCausedObjectFall(playerCells_[0], cell)) {continue;}

        uint64_t hash = (previousOption_ == currentOption_) ? tlshash::hashPath(availableOptions_, {previousOption_}) : 
            tlshash::hashPath(availableOptions_, {previousOption_, currentOption_});
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
