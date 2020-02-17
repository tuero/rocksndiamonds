/**
 * @file: engine_element_property.h
 *
 * @brief: Interface for controllers to access element property information.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


// Standard Library/STL
#include <string>

// Include
#include "engine_types.h"


namespace enginehelper {
namespace elementproperty {

    /**
     * Get a readable description name (as defined in the engine config) for the given item.
     * 
     * If the item is not known to the engine, "UNKNOWN" is returned.
     * 
     * @param item The item integer code to check.
     * @return The string readable description name.
     */
    std::string getItemReadableDescription(int item);

    /**
     * Get the number of gems the item in the grid cell counts towards.
     * 
     * @param cell The cell to check
     * @return The number of gems the item in the grid cell counts towards.
     */
    int getItemGemCount(const enginetype::GridCell cell);

    /**
     * Get the score the item in the grid cell will give the player.
     * If the grid cell is invalid, the returned score is 0.
     * 
     * @param cell The cell to check
     * @return The score the item in the cell will give the player, 0 if no score or
     * invalid cell.
     */
    int getItemScore(const enginetype::GridCell cell);

    /**
     * Check if the element in the GridCell is a temporary element.
     * A temporary element are usually for placeholders and do not represent
     * actual elements. As an example, EL_BLOCKED is used to signify an object
     * has claimed that GridCell and is currently moving into that cell.
     *
     * @param The GridCell to check
     * @return True if the element in the given cell is a temporary element.
     */
    bool isTemporaryElement(const enginetype::GridCell &cell);

    /**
     * Check if the item in the gridcell is moving.
     * 
     * Moving is defined as not being completely inside a single grid cell
     * i.e. it is moving between gridcells. This means that if an item is free
     * falling between multiple cells, there will be a single game step where is 
     * check is false.
     * 
     * @param cell The grid cell to check
     * @return True if the item in the cell is moving.
     */
    bool isMoving(const enginetype::GridCell cell);

    /**
     * Checks if the grid cell contains the exit.
     * Exit can be either open, closed, or in the process of opening/closing.
     * 
     * @param cell The grid cell to check.
     * @return True if the cell contains an exit, false otherwise.
     */
    bool isExit(const enginetype::GridCell cell);

    /**
     * Checks if the grid cell contains the open exit.
     * 
     * @param cell The grid cell to check.
     * @return True if the cell contains an open exit, false otherwise.
     */
    bool isExitOpen(const enginetype::GridCell cell);

    /**
     * Checks if the grid cell contains the closed exit.
     * 
     * @param cell The grid cell to check.
     * @return True if the cell contains a closed exit, false otherwise.
     */
    bool isExitClosed(const enginetype::GridCell cell);

    /**
     * Checks if the grid cell contains the exit which is in the 
     * process of opening.
     * 
     * @param cell The grid cell to check.
     * @return True if the cell contains an opening exit, false otherwise.
     */
    bool isExitOpening(const enginetype::GridCell cell);

    /**
     * Checks if the grid cell contains the exit which is in the 
     * process of closing.
     * 
     * @param cell The grid cell to check.
     * @return True if the cell contains a closing exit, false otherwise.
     */
    bool isExitClosing(const enginetype::GridCell cell);

    /**
     * Check if the grid cell contains a key element.
     * 
     * @param cell The grid cell to check.
     * @return True if the cell contains a key, flase otherwise.
     */
    bool isKey(const enginetype::GridCell cell);

    /**
     * Check if the grid cell contains a gate element.
     * 
     * @param cell The grid cell to check.
     * @return True if the cell contains a gate (open or closed), flase otherwise.
     */
    bool isGate(const enginetype::GridCell cell);

    /**
     * Check if the grid cell contains an unlocked gate element.
     * 
     * @param cell The grid cell to check.
     * @return True if the cell contains an unlocked gate, flase otherwise.
     */
    bool isGateOpen(const enginetype::GridCell cell);

    /**
     * Check if the grid cell contains a closed gate element.
     * 
     * @param cell The grid cell to check.
     * @return True if the cell contains a closed gate, flase otherwise.
     */
    bool isGateClosed(const enginetype::GridCell cell);

    /**
     * Checks if the direction the player wants to move in is walkable.
     * Walkable means that the player can overtop an existing element i.e. door, tube, or sokoban tiles.
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is walkable.
     */
    bool isWalkable(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if the direction the player wants to move in is walkable.
     * Walkable means that the player can overtop an existing element i.e. door, tube, or sokoban tiles (empty is also included here).
     * 
     * @note This has an extra check to ensure that the player will not be moving into a cell which 
     * will immediately end of killing the player due to a falling object above.
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is walkable.
     */
    bool isWalkableSafe(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if the direction the player wants to move in is diggable.
     * Diggable means that the player is able to stand in the cell which results
     * in the action being applied, after digging away the current element.
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is digable.
     */
    bool isDigable(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if the direction the player wants to move in is diggable.
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is empty.
     */
    bool isEmpty(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if the direction the player wants to move in is diggable.
     * 
     * @note This has an extra check to ensure that the player will not be moving into a cell which 
     * will immediately end of killing the player due to a falling object above.
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is empty.
     */
    bool isEmptySafe(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if the direction the player wants to move in is a wall.
     * This works for predefined wall objects from the default game objects. A custom
     * non-passible object (which acts as a wall) won't be caught here.
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is blocked by a wall.
     */
    bool isWall(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if the direction the player wants to move in is a rock.
     * Only rocks defined in the game are EL_ROCK or EL_BD_ROCK
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is blocked by a rock.
     */
    bool isRock(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if the player can push an element in the direction it wants to move.
     *
     * @note A pushable element is a rock.
     * 
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take will push an object.
     */
    bool isPushable(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if resulting GridCell the player wants to move to contains a collectible element.
     * 
     * @note Examples of collectible elements are gems and keys
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the resulting cell contains a collectible element
     */
    bool isCollectable(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if the direction the player wants to move in is passable.
     * Passable means that the player walks through the cell which results in the
     * action being applied, and into the next cell following the same direction.
     * 
     * @note: An example of a passble element is a tube/port.
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is passable.
     */
    bool isPassable(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if the action will move the player.
     * Player can move if they are not walking into a wall, and the GridCell in the direction
     * the player wants to move is either walkable, passable, diggable, or contains a collectable item.
     *
     * Assumes the current state to check is already set in the engine if no player cell
     * is given as the second argument.
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is passable.
     */
    bool isActionMoveable(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if the action will move the player.
     * Player can move if they are not walking into a wall, and the GridCell in the direction
     * the player wants to move is either walkable, passable, diggable, or contains a collectable item.
     *
     * @note This has an extra check to ensure that the player will not be moving into a cell which 
     * will immediately end of killing the player due to a falling object above.
     * 
     * Assumes the current state to check is already set in the engine if no player cell
     * is given as the second argument.
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is passable.
     */
    bool isActionMoveableSafe(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

}
}