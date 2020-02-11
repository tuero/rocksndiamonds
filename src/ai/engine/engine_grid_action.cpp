/**
 * @file: engine_grid_action.cpp
 *
 * @brief: Implementation of grid action information from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include "engine_helper.h"

// Standard Libary/STL
#include <array>
#include <map>
#include <cmath>                // abs

// Engine
#include "engine_types.h"       // GridCell, Action
#include "logger.h"


namespace enginehelper {
namespace gridaction {

/**
 * Check if two grid cells are neighbours, defined by being separated by Euclidean distance of 1.
 */
bool checkIfNeighbours(enginetype::GridCell left, enginetype::GridCell right) {
    return gridinfo::inBounds(left) && gridinfo::inBounds(right) && std::abs(left.x - right.x) + std::abs(left.y - right.y) == 1;
}


// Fast access mappings
const std::map<std::array<int, 2>, Action> DELTA_ACTION_MAP = 
{
    {{0,0}, Action::noop},
    {{-1,0}, Action::left},
    {{1,0}, Action::right},
    {{0,-1}, Action::up},
    {{0,1}, Action::down}
};

const std::map<Action, std::array<int, 2>> ACTION_DELTA_MAP = 
{
    {Action::noop, {0,0}},
    {Action::left, {-1,0}},
    {Action::right, {1,0}},
    {Action::up, {0,-1}},
    {Action::down, {0,1}}
};

/**
 * Get the action which moves from the first given grid cell to the second. 
 *
 * Note: The gridcells must be neighbours.
 */
Action getActionFromNeighbours(enginetype::GridCell cellFrom, enginetype::GridCell cellTo) {
    return !checkIfNeighbours(cellFrom, cellTo) ? Action::noop : DELTA_ACTION_MAP.at({cellTo.x - cellFrom.x, cellTo.y - cellFrom.y});
}


/**
 * Get the resulting cell from applying the action in the given cell.
 */
enginetype::GridCell getCellFromAction(const enginetype::GridCell cellFrom, Action directionToMove) {
    std::array<int, 2> delta = ACTION_DELTA_MAP.at(directionToMove);
    return {cellFrom.x + delta[0], cellFrom.y + delta[1]};
}

} //namespace gridaction
} //namespace enginehelper