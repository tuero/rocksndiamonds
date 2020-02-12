/**
 * @file: engine_grid_info.h
 *
 * @brief: Interface for controllers to access gridcell information from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

// ------------- Includes  -------------

// Standard Library/STL
#include <vector>

// Include
#include "engine_types.h"


namespace enginehelper {
namespace gridinfo {

    /**
     * Get L1 distance between two gridcells
     */
    int getL1Distance(const enginetype::GridCell &left, const enginetype::GridCell &right);
    
    /**
     * Checks if the given GridCell is in bounds of the level field.
     *
     * @param The GridCell to check.
     * @return True if the GridCell coordinates are valid for the map size.
     */
    bool inBounds(const enginetype::GridCell &cell);

    /**
     * Convert a gridcell to a flat index (left to right, top to bottom).
     * 
     * @param cell The gridcell to convert
     * @return Integer index representing cell location
     */
    int cellToIndex(const enginetype::GridCell &cell);

    /**
     * Convert a flat index into a gridcell.
     * 
     * @param index The index representing the gridcell
     * @return The gridcell represented by the index.
     */
    enginetype::GridCell indexToCell(int index);

    /**
     * Initalize the unique sprite IDs.
     * Temporary elements and empty space/soil do not get IDs.
     */
    void initSpriteIDs();

    /**
     * Get the sprite ID (if one exists) for the given cell.
     *
     * @param The GridCell to get the sprite.
     * @return the unique sprite ID.
     */
    int getSpriteID(const enginetype::GridCell &cell);

    /**
     * Get the grid cell the given sprite resides in.
     *
     * @param The unique ID representing the sprite.
     * @return the grid cell the sprite resides in.
     */
    enginetype::GridCell getSpriteGridCell(int spriteID);

    /**
     * Checks if the given sprite ID is active. 
     *
     * @param The unique ID of the sprite to check.
     * @return True if the sprite is still active in the level.
     */
    bool isSpriteActive(int spriteID);

    /**
     * Get the sprite locations on the level map.
     *
     * @return Vector of GridCell locations for each of the sprites.
     */
    std::vector<enginetype::GridCell> getMapSprites();

    /**
     * Get a list of every element along with some identifiable properties for the whole map.
     * 
     * @return Vector of all elements. 
     */
    std::vector<enginetype::ElementFullObservation> getFullObservation();

    /**
     * Get the item element located at the given GridCell (x,y) location.
     *
     * @param The gridcell of the element.
     * @return The integer code for the object at the grid location, or empty if GridCell out of bounds.
     */
    int getGridElement(const enginetype::GridCell &cell);

    /**
     * Get the item MovPos at the given GridCell (x,y) location.
     * This is the offset (between 0 and 32) which repsents the offset the current
     * element is on during transition from one cell to another.
     *
     * @param The gridcell of the element.
     * @return The integer code for the object offset while moving.
     */
    int getGridMovPos(const enginetype::GridCell &cell);

    /**
     * Get the item MovDir at the given GridCell (x,y) location.
     * This is the direction the object is currently moving in.
     *
     * @param The gridcell of the element.
     * @return The action direction the element is moving in.
     */
    Action getGridMovDir(const enginetype::GridCell &cell);

    /**
     * Check whether the item at location cell was just falling.
     * @note If an item is at position {x1,y1} on tick 0, starts to falls and ends up at {x1,y1+1} on tick 8, 
     *       on tick 9 getWasJustFalling({x1,y1+1}) will return true.
     * 
     * @param The gridcell of the element
     * @return True if the item in that cell was just falling on the previous tick.
     */
    bool getWasJustFalling(const enginetype::GridCell &cell);

    /**
     * Get the reserved cell that the element owns.
     * If elements are moving from one cell to another, they place a temporary element into the 
     * new cell to claim ownership. If the cell is not moving, the same cell is returned.
     * 
     * @param The gridcell of the element to check.
     * @return The gridcell the element has reserved to move into.
     */
    enginetype::GridCell getElementReservedCell(const enginetype::GridCell &cell);

    /**
     * Get the grid cell that the player is currently located in.
     *
     * @return The GridCell struct containing the player.
     */
    enginetype::GridCell getPlayerPosition();

    /**
     * Check if the player resides in the current cell.
     *
     * @param The GridCell which contains the player.
     * @return True if the player resides in the given GridCell.
     */
    bool isPlayerPosition(const enginetype::GridCell &cell);

    /**
     * Check if the grid cell at location (x,y) is empty.
     *
     * This only checks if the cell is designated by the empty integer ID, not whether
     * the grid cell is also dirt. A cell is empty if no object is currently in that
     * cell. Since it takes multiple ticks for objects to move between cells, there is 
     * also a temporary ID used for objects to claim GirdCells they are on route to travel
     * next to. 
     * 
     * @param cell The GridCell struct to check if empty.
     * @return True if the GirdCell is designated by the empty int code.
     */
    bool isGridEmpty(const enginetype::GridCell &cell);

    /**
     * Get all grid cells which are empty.
     *
     * @return A vector of GridCells structs which do not have an object in it.
     */
    std::vector<enginetype::GridCell> getEmptyGridCells();

    /**
     * Count how many of a specified element in the game.
     *
     * @param element The integer ID of the element to count.
     * @return The count of the element.
     */
    int countNumOfElement(int element);

    /**
     * Add the specified element to the level.
     *
     * @param element The element integer ID to spawn.
     * @param dir The movement direciton ID the object should start moving in.
     * @param gridCell The grid cell the element should spawn in.
     */
    void spawnElement(int element, int dir, const enginetype::GridCell &cell);


} //namespace gridinfo
} //namespace enginehelper