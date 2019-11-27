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

// STL containers
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <limits>

// Engine
#include "../../engine/engine_helper.h"

// Logger
#include "../util/logger.h"
#include <plog/Logger.h>


/**
 * Determine if the option is valid to perform as per the current game engine state.
 */
bool BaseOption::isValid() {
    return isValid_() && enginehelper::isSpriteActive(spriteID_);
    // return isValid_();
}

bool BaseOption::isComplete() {
    return false;
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
 * Set the Options solutionPath_ as the path found during A* 
 * at the grid level (time-independent).
 */
void BaseOption::runAStar() {
    runAStar(enginehelper::getPlayerPosition(), goalCell_);
}


/**
 * Set the Options solutionPath_ as the path found during A* 
 * at the grid level (time-independent).
 */
void BaseOption::runAStar(enginetype::GridCell startCell, enginetype::GridCell goalCell) {
    Node startNode = {enginehelper::cellToIndex(startCell), -1, startCell, 0, (float)enginehelper::getL1Distance(startCell, goalCell)};

    PLOGV_(logger::FileLogger) << "Running A* at grid cell level";
    PLOGV_(logger::FileLogger) << "Start node: x= " << startCell.x << ", y= " << startCell.y;
    PLOGV_(logger::FileLogger) << "Goal node: x= " << goalCell.x << ", y= " << goalCell.y;

    // A* data structures
    std::unordered_map<int, Node> open;
    std::unordered_map<int, Node> closed;

    // Initialize with start node
    open[startNode.id] = startNode;
    solutionPath_.clear();

    while (!open.empty()) {
        // Pull next node and update data structures
        Node node = {-1, -1, {-1, -1}, std::numeric_limits<float>::max(), 0};
        for (auto it = open.begin(); it != open.end(); ++it) {
            if (it->second.g + it->second.h < node.g + node.h) {
                node = it->second;
            }
        }

        PLOGV_(logger::FileLogger) << "Pulling node: x= " << node.cell.x << ", y= " << node.cell.y << ", id " << node.id << ", pid " << node.parentId;

        if (node.g == std::numeric_limits<float>::max()) {
            PLOGE_(logger::ConsoleLogger) << "Cannot find a node to pull";
            PLOGE_(logger::FileLogger) << "Cannot find a node to pull";
            break;
        }

        open.erase(node.id);
        closed[node.id] = node;

        // Goal condition check
        if (node.cell == goalCell) {
            PLOGV_(logger::FileLogger) << "Found solution";
            solutionPath_.clear();
            while(!(node.cell == startCell)) {
                PLOGV_(logger::FileLogger) << "node: x= " << node.cell.x << ", y= " << node.cell.y << ", id " << node.id << ", pid " << node.parentId;
                solutionPath_.push_front(node.cell);
                node = (closed.find(node.parentId) != closed.end() ? closed[node.parentId] : open[node.parentId]);
            }
            return;
        }

        // Expand children
        for (Action action : ALL_ACTIONS_NO_NOOP) {
            // Skip if in the restricted set
            enginetype::GridCell childCell = enginehelper::getCellFromAction(action, node.cell);
            if (std::find(restrictedCells_.begin(), restrictedCells_.end(), childCell) != restrictedCells_.end()) {continue;}

            // Child not valid if out of bounds or action doesn't result in being in a moveable cell
            if (!enginehelper::isActionMoveable(action, node.cell) && !(childCell == goalCell)) {continue;}

            int childIndex = enginehelper::cellToIndex(childCell);
            float newG = node.g + 1;
            float h = (float)enginehelper::getL1Distance(childCell, goalCell);

            // Node generated but not expanded
            if (open.find(childIndex) != open.end()) {
                // Check if new path cheaper
                if (open[childIndex].g < newG) {continue;}
                PLOGV_(logger::FileLogger) << "Removing from open: x= " << open[childIndex].cell.x << ", y= " << open[childIndex].cell.y << ", id " << open[childIndex].id << ", pid " << open[childIndex].parentId;
                open.erase(childIndex);
            }
            // Node already expanded
            else if (closed.find(childIndex) != closed.end()) {
                // Check if new path cheaper
                if (closed[childIndex].g < newG) {continue;}
                PLOGV_(logger::FileLogger) << "Removing from closed: x= " << closed[childIndex].cell.x << ", y= " << closed[childIndex].cell.y << ", id " << closed[childIndex].id << ", pid " << closed[childIndex].parentId;
                closed.erase(childIndex);
            }

            PLOGV_(logger::FileLogger) << "Adding to open: x= " << childCell.x << ", y= " << childCell.y << ", id " << childIndex << ", pid " << node.id;
            open[childIndex] = {childIndex, node.id, childCell, newG, h};
        }
    }
}