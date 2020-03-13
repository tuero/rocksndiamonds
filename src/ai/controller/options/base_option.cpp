/**
 * @file: base_option.cpp
 *
 * @brief: Base option which all implemented options should be derived from.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#include "base_option.h"

// Standard Libary/STL
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <set>

// Includes
#include "engine_helper.h"
#include "logger.h"

using namespace enginehelper;


/**
 * Reset the option.
 */
void BaseOption::reset() {
    // Find sprite ID again
    goalCell_ = gridinfo::getSpriteGridCell(spriteID_);
    counter_ = 0;
    solutionPath_.clear();
    restrictedCells_.clear();
}


/**
 * Get the option type.
 */
OptionType BaseOption::getOptionType() const {
    return optionType_;
}

/**
 * Increment the number of times the option has been expanded during search.
 */
void BaseOption::incrementTimesCalled() {
    timesCalled_ += 1;
}

/**
 * Reset the number of times the option has been expanded during search.
 */
void BaseOption::resetTimesCalled() {
    timesCalled_ = 0;
}


/**
 * Get the number of times the option has been expanded during search.
 */
int BaseOption::getTimesCalled() const {
    return timesCalled_;
}


/**
 * Set the sprite ID that the option represents/interacts with.
 */
void BaseOption::setSpriteID(int spriteID) {
    spriteID_ = spriteID;
}


/**
 * Get the sprite ID that the option represents/interacts with.
 */
int BaseOption::getSpriteID() const {
    return spriteID_;
}


/**
 * Get the solution path as found by A*.
 */
std::deque<enginetype::GridCell> BaseOption::getSolutionPath() {
    return solutionPath_;
}

/**
 * Set the restricted cells.
 */
void BaseOption::setRestrictedCells(std::vector<enginetype::GridCell> &restrictedCells) {
    restrictedCells_ = restrictedCells;
}

/**
 * Set the flag which indicates whether A* will avoid collectible elements which are
 * not the current goal location.
 */
void BaseOption::setAvoidNonGoalCollectibleCells(bool flag) {
    avoidNonGoalCollectibleCells = flag;
}

/**
 * Set the flag which indicates whether A* will avoid cells which can cause the
 * player to explode.
 */
void BaseOption::setPrioritizeSafeCells(bool flag) {
    prioritizeSafeCells = flag;
}


/**
 * Set the restricted cells.
 */
void BaseOption::setRestrictedCells(std::unordered_set<int> &restrictedCells) {
    restrictedCells_.clear();
    for (auto const & cell : restrictedCells) {
        restrictedCells_.push_back(gridinfo::indexToCell(cell));
    }
}

/**
 * Get the list of restricted cells.
 */
const std::vector<enginetype::GridCell> & BaseOption::getRestrictedCells() {
    return restrictedCells_;
}


/**
 * Clear the saved restricted cells.
 */
void BaseOption::clearRestrictedCells() {
    restrictedCells_.clear();
}


/**
 * Add a restricted cell.
 */
void BaseOption::addRestrictedCell(enginetype::GridCell &cell) {
    restrictedCells_.push_back(cell);
}


/**
 * Add a restricted cell.
 */
void BaseOption::addRestrictedCell(int index) {
    restrictedCells_.push_back(gridinfo::indexToCell(index));
}

/**
 * Get the current goal cell.
 */
enginetype::GridCell BaseOption::getGoalCell() const {
    return goalCell_;
}


/**
 * Set the Options solutionPath_ as the path found during A* 
 * at the grid level (time-independent).
 */
void BaseOption::runAStar() {
    runAStar(gridinfo::getPlayerPosition(), goalCell_);
}


// Node for A* search
struct Node {
    int id;                             // Fast access node ID = gridcell index 
    int parentId;                       // node ID for the parent node
    enginetype::GridCell cell;          // Gridcell represented by search node
    float g;                            // g-value used in A*
    float h;                            // h-value used in A* (Euclidean distance)
};

// Custom comparator for priority queue 
struct CompareNode {
    bool operator() (Node* left, Node* right) const {
        float vleft = left->g + left->h;
        float vright = right->g + right->h;
        return vleft < vright || (vleft == vright && left->id < right->id);
    }
};


bool canExpandAvoidCollectingNonGoalGems(const enginetype::GridCell &cellFrom, const enginetype::GridCell &cellTo, 
    const enginetype::GridCell &goalCell, Action action, bool prioritizeSafeCells) 
{
    bool is_empty = (prioritizeSafeCells) ? elementproperty::isEmptySafe(cellFrom, action) : elementproperty::isEmpty(cellFrom, action);
    bool is_walkable = (prioritizeSafeCells) ? elementproperty::isWalkableSafe(cellFrom, action) : elementproperty::isWalkable(cellFrom, action);
    return elementproperty::isDigable(cellFrom, action) || is_walkable || is_empty ||
        elementproperty::isGateOpen(cellTo) || elementproperty::isGateOpen(cellTo) || cellTo == goalCell;
}

bool canExpandCollectingNonGoalGems(const enginetype::GridCell &cellFrom, const enginetype::GridCell &cellTo, 
    const enginetype::GridCell &goalCell, Action action, bool prioritizeSafeCells) 
{
    bool is_moveable = (prioritizeSafeCells) ? elementproperty::isActionMoveableSafe(cellFrom, action) : elementproperty::isActionMoveable(cellFrom, action);
    return is_moveable || elementproperty::isGateOpen(cellTo) || cellTo == goalCell;
}


/**
 * Set the Options solutionPath_ as the path found during A* 
 * at the grid level (time-independent).
 */
void BaseOption::runAStar(enginetype::GridCell startCell, enginetype::GridCell goalCell) {
    Node startNode = {gridinfo::cellToIndex(startCell), -1, startCell, 0, (float)gridinfo::getL1Distance(startCell, goalCell)};

    auto expandFunc = (avoidNonGoalCollectibleCells) ? canExpandAvoidCollectingNonGoalGems : canExpandCollectingNonGoalGems;
    // PLOGE_(logger::FileLogger) << "start: x=" << startNode.cell.x <<  ", y=" << startNode.cell.y;
    // PLOGE_(logger::FileLogger) << "goal: x=" << goalCell.x <<  ", y=" << goalCell.y;

    // A* data structures
    std::unordered_map<int, Node> open;
    std::set<Node*, CompareNode> openSet;
    std::unordered_map<int, Node> closed;

    // Initialize with start node
    open[startNode.id] = startNode;
    openSet.insert(&open[startNode.id]);
    solutionPath_.clear();

    while (!openSet.empty()) {
        // Pull next node and update data structures
        Node node = open[(*openSet.begin())->id];
        openSet.erase(openSet.begin());
        open.erase(node.id);
        closed[node.id] = node;

        // Goal condition check
        if (node.cell == goalCell) {
            solutionPath_.clear();
            while(!(node.cell == startCell)) {
                solutionPath_.push_front(node.cell);
                node = (closed.find(node.parentId) != closed.end() ? closed[node.parentId] : open[node.parentId]);
            }
            return;
        }

        // Expand children
        for (Action action : enginetype::ALL_ACTIONS_NO_NOOP) {
            // Skip if in the restricted set
            enginetype::GridCell childCell = gridaction::getCellFromAction(node.cell, action);
            if (std::find(restrictedCells_.begin(), restrictedCells_.end(), childCell) != restrictedCells_.end()) {continue;}

            // Child not valid if out of bounds or action doesn't result in being in a moveable cell
            if (!expandFunc(node.cell, childCell, goalCell, action, prioritizeSafeCells)) {continue;}

            int childIndex = gridinfo::cellToIndex(childCell);
            float newG = node.g + 1;
            float h = (float)gridinfo::getL1Distance(childCell, goalCell);

            // Node generated but not expanded
            if (open.find(childIndex) != open.end()) {
                // Check if new path cheaper
                if (open[childIndex].g <= newG) {continue;}
                openSet.erase(&open[childIndex]);
                open.erase(childIndex);
            }
            // Node already expanded
            else if (closed.find(childIndex) != closed.end()) {
                // Check if new path cheaper
                if (closed[childIndex].g <= newG) {continue;}
                closed.erase(childIndex);
            }

            Node childNode{childIndex, node.id, childCell, newG, h};
            open[childIndex] = childNode;
            openSet.insert(&open[childIndex]);
        }
    }

    // A* is usually called to check if an option is valid (can we path to it), so its not needed
    // to throw errors if we fail.
    PLOGE_(logger::FileLogger) << "A* couldn't find a path";
}
