/**
 * @file: engine_grid_info.cpp
 *
 * @brief: Implementation of gridcell information from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include "engine_helper.h"

// Standard Libary/STL
#include <vector>
#include <unordered_map>

// Includes
#include "engine_types.h"       // GridCell, Action

// Game headers
extern "C" {
    #include "../../main.h"                 // level, Feld
}


extern int spriteIDs[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int spriteIDCounter;

namespace enginehelper {
namespace gridinfo {


/**
 * Get L1 distance between two gridcells
 */
int getL1Distance(const enginetype::GridCell &left, const enginetype::GridCell &right) {
    return (std::abs(left.x - right.x) + std::abs(left.y - right.y));
}

/**
 * Checks if the given gridcell is in bounds of the level field.
 */
bool inBounds(const enginetype::GridCell &cell) {
    return (cell.x >= 0 && cell.x < level.fieldx && cell.y >= 0 && cell.y < level.fieldy);
}


/**
 * Convert a gridcell to a flat index
 */
int cellToIndex(const enginetype::GridCell &cell) {
    return inBounds(cell) ? (cell.y * levelinfo::getLevelWidth() + cell.x) : -1;
}


/**
 * Convert a flat index into a gridcell.
 */
enginetype::GridCell indexToCell(int index) {
    const enginetype::GridCell invalidCell = {-1, -1};
    if (index == -1) {return invalidCell;}
    int x = index % levelinfo::getLevelWidth();
    int y = index / levelinfo::getLevelWidth();
    return (x >= levelinfo::getLevelWidth() || y >= levelinfo::getLevelHeight()) ? invalidCell : (enginetype::GridCell ){x, y};
}


/**
 * Checks if a given cell contains a valid sprite.
 * For internal use when bound-check is implicitly done.
 */
bool _isValidSprite(const enginetype::GridCell &cell) {
    return !elementproperty::isTemporaryElement(cell) && !elementproperty::isEmpty(cell);
}


/**
 * Initialize the unique sprite IDs
 */
void initSpriteIDs() {
    spriteIDCounter = 0;

    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            spriteIDs[x][y] = -1;
            enginetype::GridCell cell = {x, y};
            // Non-player non-temporary sprite
            if (!isPlayerPosition(cell) && _isValidSprite(cell)) {
                spriteIDs[x][y] = spriteIDCounter++;
            }
        }
    }
}


/**
 * Get the sprite ID (if one exists) for the given cell.
 */
int getSpriteID(const enginetype::GridCell &cell) {
    return inBounds(cell) ? spriteIDs[cell.x][cell.y] : -1;
}


/**
 * Get the grid cell the given sprite resides in.
 */
enginetype::GridCell getSpriteGridCell(int spriteID) {
    enginetype::GridCell cell = {-1, -1};
    if (spriteID == -1) {return cell;}
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            if (spriteIDs[x][y] == spriteID) {
                cell.x = x;
                cell.y = y;
                break;
            }
        }
    }
    return cell;
}


/**
 * Checks if the given sprite ID is active. 
 */
bool isSpriteActive(int spriteID) {
    // Non-identifiable sprite
    if (spriteID == -1) {return false;}
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            if (spriteIDs[x][y] == spriteID) {return true;}
        }
    }
    return false;
}


/**
 * Get the sprite locations on the level map.
 */
std::vector<enginetype::GridCell> getMapSprites() {
    std::vector<enginetype::GridCell> mapSprites;

    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            enginetype::GridCell cell = {x, y};
            if (!isPlayerPosition(cell) && _isValidSprite(cell)) {
                mapSprites.push_back({x,y});
            }
        }
    }
    return mapSprites;
}


/**
 * Get a list of every element along with some identifiable properties for the whole map.
 */
std::vector<enginetype::ElementFullObservation> getFullObservation() {
    std::vector<enginetype::ElementFullObservation> obs;
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            enginetype::GridCell cell = {x, y};
            obs.push_back({cell, getElementReservedCell(cell), getSpriteID(cell), 
                (isPlayerPosition(cell)) ? enginetype::ELEMENT_PLAYER_1 : getGridElement(cell), getGridMovDir(cell)
            });
        }
    }
    return obs;
}


/**
 * Get the item element located at the given GridCell (x,y) location.
 */
int getGridElement(const enginetype::GridCell &cell) {
    return inBounds(cell) ? Feld[cell.x][cell.y] : EL_EMPTY;
}


/**
 * Get the item MovPos at the given GridCell (x,y) location.
 * This is the offset (between 0 and 32) which repsents the offset the current
 * element is on during transition from one cell to another.
 */
int getGridMovPos(const enginetype::GridCell &cell) {
    return inBounds(cell) ? MovPos[cell.x][cell.y] : 0;
}


static const std::unordered_map<int, Action> DIR_TO_ACTION = 
{
    {enginetype::ENGINE_MV_RIGHT,   Action::right},
    {enginetype::ENGINE_MV_DOWN,    Action::down},
    {enginetype::ENGINE_MV_LEFT,    Action::left},
    {enginetype::ENGINE_MV_UP,      Action::up},
    {enginetype::ENGINE_MV_NONE,    Action::noop}
};

/**
 * Get the item MovDir at the given GridCell (x,y) location.
 * This is the direction the object is currently moving in.
 */
Action getGridMovDir(const enginetype::GridCell &cell) {
    return inBounds(cell) ? DIR_TO_ACTION.at(MovDir[cell.x][cell.y]) : Action::noop;
}


/**
 * Check whether the item which was previously at location cell was just falling.
 */
bool getWasJustFalling(const enginetype::GridCell &cell) {
    return inBounds(cell) ? WasJustFalling[cell.x][cell.y] == 3 : false;
}


/**
 * Get the reserved cell that the element owns.
 * If elements are moving from one cell to another, they place a temporary element into the 
 * new cell to claim ownership. If the cell is not moving, the same cell is returned.
 */
enginetype::GridCell getElementReservedCell(const enginetype::GridCell &cell) {
    return (getGridMovPos(cell) == 0) ? cell : gridaction::getCellFromAction(cell, getGridMovDir(cell));
}


/**
 * Get the grid cell that the player is currently located in.
 */
enginetype::GridCell getPlayerPosition() {
    enginetype::GridCell gridCell = {stored_player[0].jx, stored_player[0].jy};
    return gridCell;
}


/**
 * Check if the player resides in the current cell.
 */
bool isPlayerPosition(const enginetype::GridCell &cell) {
    return (cell.x == stored_player[0].jx && cell.y == stored_player[0].jy);
}


/**
 * Check if the grid cell at location (x,y) is empty.
 */
bool isGridEmpty(const enginetype::GridCell &cell) {
    // If not in bounds, occupied by player or temporary element then false
    // Then check if Feld structure at cell corresponds to empty sprite
    return (!inBounds(cell) || isPlayerPosition(cell) || elementproperty::isTemporaryElement(cell)) ? false : Feld[cell.x][cell.y] == EL_EMPTY;
}


/**
 * Get all grid cells which are empty.
 */
std::vector<enginetype::GridCell> getEmptyGridCells() {
    std::vector<enginetype::GridCell> emptyGridCells;
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (isGridEmpty({x, y})) {
                emptyGridCells.push_back(enginetype::GridCell{x, y});
            }
        }
    }
    return emptyGridCells;
}


/**
 * Count how many of a specified element in the game.
 */
int countNumOfElement(int element) {
    int elementCount = 0;
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (Feld[x][y] == element) {
                elementCount += 1;
            }
        }
    }
    return elementCount;
}

/**
 * Add the specified element to the level.
 */
void spawnElement(int element, int dir, const enginetype::GridCell &cell) {
    if (!inBounds(cell)) {
        return;
    }
    Feld[cell.x][cell.y] = element;
    MovDir[cell.x][cell.y] = dir;
}


} //namespace gridinfo
} //namespace enginehelper