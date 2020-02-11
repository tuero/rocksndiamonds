/**
 * @file: engine_grid_action.h
 *
 * @brief: Interface for controllers to access grid action information from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

// ------------- Includes  -------------

// Include
#include "engine_types.h"


namespace enginehelper {
namespace gridaction {

    /**
     * Check if two grid cells are neighbours, defined by being separated by Euclidean distance of 1.
     *
     * @param left First GridCell struct to compare.
     * @param left Second GridCell struct to compare.
     * @return True if the two GridCells are neighbours.
     */
    bool checkIfNeighbours(enginetype::GridCell left, enginetype::GridCell right);

    /**
     * Get the action which moves from the first given grid cell to the second. 
     *
     * Note: The gridcells must be neighbours.
     *
     * @param cellFrom The starting gridcell.
     * @param cellTo The ending gridcell.
     * @param The action which resulted in going from gridcell from to gridcell to.
     */
    Action getActionFromNeighbours(enginetype::GridCell cellFrom, enginetype::GridCell cellTo);

   /**
    * Get the resulting cell from applying the action in the given cell.
    * 
    * @param cellFrom The reference GridCell.
    * @param directionToMove The action to apply.
    * @return The resulting GridCell after applying the given action.
    */
    enginetype::GridCell getCellFromAction(const enginetype::GridCell cellFrom, Action directionToMove);

} //namespace gridaction
} //namespace enginehelper