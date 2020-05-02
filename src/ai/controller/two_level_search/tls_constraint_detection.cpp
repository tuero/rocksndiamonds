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


/**
 * If we are using manual constraints (less accurate but faster loop throughput), we need to try and guess
 * what the constraints would be. Here, we look for rock positions, between high level options (sprites)
 * that would occur from walking along shortest path.
 */
void TwoLevelSearch::addManualConstraints() {
    // Find rocks (potential restrictions)
    std::vector<enginetype::GridCell> rockCells;
    PLOGE_(logger::FileLogger) << "Rock restrictions";
    for (auto const & cell : gridinfo::getMapSprites()) {
        if (gridinfo::getGridElement(cell) == enginetype::ELEMENT_BD_ROCK) {
            rockCells.push_back({cell.x, cell.y + 1});
            PLOGE_(logger::FileLogger) <<  cell.x << "," << cell.y+1;
        }
    }

    // Agent starting position to each highlevel action
    PLOGE_(logger::FileLogger) << "Setting restrictions for single step path (agent starting position -> first option)";
    enginetype::GridCell startCell = gridinfo::getPlayerPosition();
    for (auto const & option : availableOptions_) {
        uint64_t hash = tlshash::hashPath(availableOptions_, {option});
        option->runAStar(startCell, option->getGoalCell());
        // Walk along base path and if a rock is above, set as restriction
        for (auto const & cell :option->getSolutionPath()) {
            if (std::find(rockCells.begin(), rockCells.end(), cell) != rockCells.end()) {
                restrictedCellsByOption_[hash].insert(gridinfo::cellToIndex(cell));
                ++restrictedCellsByOptionCount_[hash];
                PLOGE_(logger::FileLogger) << option->toString() << ", (" << cell.x << ", " << cell.y << ")";
            }
        }
    }

    // Pair options
    PLOGE_(logger::FileLogger) << "Setting restrictions for pair of options";
    for (auto & prev : availableOptions_) {
        for (auto & curr : availableOptions_) {
            // Same option means starting position to option, handled above
            if (prev == curr) {continue;}
            uint64_t hash = tlshash::hashPath(availableOptions_, {prev, curr});
            
            // Set path
            curr->runAStar(prev->getGoalCell(), curr->getGoalCell());

            // Walk along base path and if a rock is above, set as restriction
            for (auto const & cell :curr->getSolutionPath()) {
                if (std::find(rockCells.begin(), rockCells.end(), cell) != rockCells.end()) {
                    restrictedCellsByOption_[hash].insert(gridinfo::cellToIndex(cell));
                    ++restrictedCellsByOptionCount_[hash];
                    PLOGE_(logger::FileLogger) << prev->toString() << " -> " << curr->toString() << ", (" << cell.x << ", " << cell.y << ")";
                }
            }
        }
    }
}


/**
 * For each HLA pair, insert cell restrictions into a set for the whole path
 * If a restriction occurs twice in the path (once at different steps in the path),
 * then we only consider it once
 */
void TwoLevelSearch::setPathRestrictionSet(uint64_t pathHash, const std::vector<BaseOption*> &path) {
    restrictedCellsByPath_[pathHash].clear();
    for (auto const & pairHash : tlshash::pathToPairHashes(availableOptions_, path)) {
        for (auto const & r : restrictedCellsByOption_[pairHash]) {
            if (restrictedCellsByPath_[pathHash].find(r) == restrictedCellsByPath_[pathHash].end()) {
                restrictedCellsByPath_[pathHash].insert(r);
            }
        }
    }
}


/**
 * Get the restriction count for the given path
 */
int TwoLevelSearch::restrictionCountForPath(const std::vector<BaseOption*> &path) {
#ifndef SET_RESTRICTIONS
    std::vector<uint64_t> nodePathHashes = tlshash::pathToPairHashes(availableOptions_, path);
    auto lambda = [&](int sum, uint64_t hash) {return sum + restrictedCellsByOptionCount_[hash];};
    return std::accumulate(nodePathHashes.begin(), nodePathHashes.end(), 0, lambda);
#else
    uint64_t pathHash = tlshash::hashPath(availableOptions_, path);
    setPathRestrictionSet(pathHash, path);
    return restrictedCellsByPath_[pathHash].size();
#endif
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
        // Otherwise, new constraint seen affects the node in open. 
        // Update constraints for that node and reset counter back to 0
        int totalConstraintCount = restrictionCountForPath(it->path);

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


/**
 * Check for newely moved objects as a result of player actions.
 * Objects are stored as sprite and gridcell pairs for a given pair of options (option from -> option to)
 * In addNewConstraints(), we store only the intersection of gridcells in restrictedCellsByOption_
 */
void TwoLevelSearch::checkForMovedObjects() {
    // Increment player buffered positions
    if (gridinfo::getPlayerPosition() != playerCells_[1]) {
        playerCells_[0] = playerCells_[1];
        playerCells_[1] = gridinfo::getPlayerPosition();
    }

    // First tick, no movement, break early
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
