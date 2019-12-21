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
#include <array>

// Includes
#include "engine_types.h"       // GridCell, Action

// Game headers
extern "C" {
    #include "../../main.h"                 // level, Feld
}


extern int spriteIDs[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int spriteIDCounter;

namespace enginehelper {

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
    return inBounds(cell) ? (cell.y * getLevelWidth() + cell.x) : -1;
}


/**
 * Checks if a given cell contains a valid sprite.
 * For internal use when bound-check is implicitly done.
 */
bool _isValidSprite(const enginetype::GridCell &cell) {
    int element = Feld[cell.x][cell.y];
    bool isRestricted = enginetype::RESTRICTED_SPRITES.find(element) != enginetype::RESTRICTED_SPRITES.end();
    return !isRestricted && !isTemporaryElement(cell);
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
 * Get the item element located at the given GridCell (x,y) location.
 */
int getGridElement(enginetype::GridCell cell) {
    return inBounds(cell) ? Feld[cell.x][cell.y] : EL_EMPTY;
}


/**
 * Get the item MovPos at the given GridCell (x,y) location.
 */
int getGridMovPos(enginetype::GridCell cell) {
    return inBounds(cell) ? MovPos[cell.x][cell.y] : 0;
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
bool isGridEmpty(enginetype::GridCell cell) {
    // If not in bounds, occupied by player or temporary element then false
    // Then check if Feld structure at cell corresponds to empty sprite
    return (!inBounds(cell) || isPlayerPosition(cell) || isTemporaryElement(cell)) ? false : Feld[cell.x][cell.y] == EL_EMPTY;
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
void spawnElement(int element, int dir, enginetype::GridCell gridCell) {
    if (!inBounds(gridCell)) {
        return;
    }
    Feld[gridCell.x][gridCell.y] = element;
    MovDir[gridCell.x][gridCell.y] = dir;
}


/**
 * Get the current goal location (defined by distance of 0).
 */
enginetype::GridCell getCurrentGoalLocation() {
    enginetype::GridCell gridCell{-1, -1};
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            if (distances[x][y] == 0) {gridCell.x = x; gridCell.y = y;}
        }
    }
    return gridCell;
}

} //namespace enginehelper