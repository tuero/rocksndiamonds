/**
 * @file: engine_distance.cpp
 *
 * @brief: Implementation of distance functions from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include "engine_helper.h"

// Standard Libary/STL
#include <cmath>                // abs

// Includes
#include "engine_types.h"       // GridCell, Action

// Game headers
extern "C" {
    #include "../../main.h"     // distances struct
}


namespace enginehelper {

/**
 * Get L1 distance between two gridcells
 */
int getL1Distance(enginetype::GridCell left, enginetype::GridCell right) {
    return (std::abs(left.x - right.x) + std::abs(left.y - right.y));
}


/**
 * Get the distance to goal from given GridCell
 * This is set by distance metric, usually L1
 */
int getGridDistanceToGoal(const enginetype::GridCell cell) {
    if (!inBounds(cell)) {
        return -1;
    }
    return distances[cell.x][cell.y];
}


/**
 * Set the distance to goal manually for more exotic distance functions.
 */
void setGridDistanceToGoal(const enginetype::GridCell cell, short value) {
    if (!inBounds(cell)) {
        return;
    }
    distances[cell.x][cell.y] = value;
}


/**
 * Find the grid location of the goal, given by a distance measure of 0.
 */
enginetype::GridCell findDistanceGoalCell() {
    enginetype::GridCell goal_cell = {-1, -1};
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (distances[x][y]==0) {
                goal_cell.x = x;
                goal_cell.y = y;
                return goal_cell;
            }
        }
    }
    return goal_cell;
}


/**
 * Find the grid location of the goal, given by EL_EXIT_OPEN
 */
enginetype::GridCell findExitLocation() {
    enginetype::GridCell goal_cell = {-1, -1};
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (enginehelper::isExit({x, y})) {
                goal_cell.x = x;
                goal_cell.y = y;
                return goal_cell;
            }
        }
    }
    return goal_cell;
}


/**
 * Reset the board distances.
 */
void resetBoardDistances() {
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            distances[x][y] = -1;
        }
    }
}


/**
 * Set the grid distances to goal using L1 distance
 */
void setBoardDistancesL1(const enginetype::GridCell goalCell) {
    // Reset distances
    resetBoardDistances();

    // Goal cell not explicitly given, so try to find the exit location.
    enginetype::GridCell goalCell_ = (inBounds(goalCell)) ? goalCell : findExitLocation();
    if (!inBounds(goalCell_)) {
        return;
    }

    // Set goal distance
    distances[goalCell_.x][goalCell_.y] = 0;

    // Set other grid distances
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (!isWall({x, y})) {
                distances[x][y] = std::abs(goalCell_.x - x) + std::abs(goalCell_.y - y);
            }
        }
    }
}

} //namespace enginehelper