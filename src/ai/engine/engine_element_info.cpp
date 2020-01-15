/**
 * @file: engine_element_info.cpp
 *
 * @brief: Implementation of element property information from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include "engine_helper.h"

// Standard Libary/STL
#include <string>

// Includes
#include "engine_types.h"       // GridCell, Action
#include "logger.h"

// Game headers
extern "C" {
    #include "../../main.h"                 // element_name_info, Feld, engine constants
}


namespace enginehelper {


/**
 * Get a readable description name (as defined in the engine config) for the given item.
 */
std::string getItemReadableDescription(int item) {
    try{
        std::string elementDescription(element_name_info[item].editor_description);
        return elementDescription;
    }
    catch (...) {
        PLOGE_(logger::FileLogger) << "Unknown item type: " << item;
    }
    return "UNKNOWN";
}


/**
 * Get the number of gems the item in the grid cell counts towards.
 */
int getItemGemCount(const enginetype::GridCell cell) {
    int item = getGridElement(cell);
    return element_info[item].collect_count_initial;
}


/**
 * Get the score the item in the grid cell will give the player.
 * If the grid cell is invalid, the returned score is 0.
 */
int getItemScore(const enginetype::GridCell cell) {
    int item = getGridElement(cell);
    int score = 0;

    if (item == EL_EMPTY) {return score;}

    switch (item) {
        case EL_EMERALD :
        case EL_BD_DIAMOND :
        case EL_EMERALD_YELLOW :
        case EL_EMERALD_RED :
        case EL_EMERALD_PURPLE :
        case EL_SP_INFOTRON :
            score = level.score[SC_EMERALD];
            break;
        case EL_DIAMOND :
            score = level.score[SC_DIAMOND];
            break;
        case EL_CRYSTAL :
            score = level.score[SC_CRYSTAL];
            break;
        case EL_PEARL :
            score = level.score[SC_PEARL];
            break;
        case EL_BUG :
        case EL_BD_BUTTERFLY :
        case EL_SP_ELECTRON :
            score = level.score[SC_BUG];
            break;
        case EL_SPACESHIP :
        case EL_BD_FIREFLY :
        case EL_SP_SNIKSNAK :
            score = level.score[SC_SPACESHIP];
            break;
        case EL_YAMYAM :
        case EL_DARK_YAMYAM :
            score = level.score[SC_YAMYAM];
            break;
        case EL_ROBOT :
            score = level.score[SC_ROBOT];
            break;
        case EL_PACMAN :
            score = level.score[SC_PACMAN];
            break;
        case EL_NUT :
            score = level.score[SC_NUT];
            break;
        case EL_DYNAMITE :
        case EL_EM_DYNAMITE :
        case EL_SP_DISK_RED :
        case EL_DYNABOMB_INCREASE_NUMBER :
        case EL_DYNABOMB_INCREASE_SIZE :
        case EL_DYNABOMB_INCREASE_POWER :
            score = level.score[SC_DYNAMITE];
            break;
        case EL_SHIELD_NORMAL :
        case EL_SHIELD_DEADLY :
            score = level.score[SC_SHIELD];
            break;
        case EL_EXTRA_TIME :
            score = level.extra_time_score;
            break;
        case EL_KEY_1 :
        case EL_KEY_2 :
        case EL_KEY_3 :
        case EL_KEY_4 :
        case EL_EM_KEY_1 :
        case EL_EM_KEY_2 :
        case EL_EM_KEY_3 :
        case EL_EM_KEY_4 :
        case EL_EMC_KEY_5 :
        case EL_EMC_KEY_6 :
        case EL_EMC_KEY_7 :
        case EL_EMC_KEY_8 :
        case EL_DC_KEY_WHITE :
            score = level.score[SC_KEY];
            break;
        default :
            score = element_info[item].collect_score;
            break;
    }

    return score;
}


/**
 * Check if the element in the gridcell is a temporary element.
 */
bool isTemporaryElement(const enginetype::GridCell &cell) {
    return inBounds(cell) && Feld[cell.x][cell.y] >= NUM_DRAWABLE_ELEMENTS;
}


/**
 * Check if the item in the gridcell is moving.
 * 
 * Moving is defined as not being completely inside a single grid cell
 * i.e. it is moving between gridcells. This means that if an item is free
 * falling between multiple cells, there will be a single game step where is 
 * check is false.
 */
bool isMoving(const enginetype::GridCell cell) {
    return inBounds(cell) && IS_MOVING(cell.x, cell.y);
}


/**
 * Checks if the grid cell contains the exit.
 * Exit can be either open, closed, or in the process of opening/closing.
 */
bool isExit(const enginetype::GridCell cell) {
    int item = getGridElement(cell);
    return item == EL_EXIT_CLOSED || item == EL_EXIT_OPEN || item == EL_EXIT_OPENING || item == EL_EXIT_CLOSING;
}


/**
 * Checks if the grid cell contains the open exit.
 */
bool isExitOpen(const enginetype::GridCell cell) {
    int item = getGridElement(cell);
    return item == EL_EXIT_OPEN;
}


/**
 * Checks if the grid cell contains the closed exit.
 */
bool isExitClosed(const enginetype::GridCell cell) {
    int item = getGridElement(cell);
    return item == EL_EXIT_CLOSED;
}


/**
 * Checks if the grid cell contains the exit which is in the 
 * process of opening.
 */
bool isExitOpening(const enginetype::GridCell cell) {
    int item = getGridElement(cell);
    return item == EL_EXIT_OPENING;
}


/**
 * Checks if the grid cell contains the exit which is in the 
 * process of closing.
 */
bool isExitClosing(const enginetype::GridCell cell) {
    int item = getGridElement(cell);
    return item == EL_EXIT_CLOSING;
}


/**
 * Check if given gridcell is accessible by moving in the given direction.
 */
bool _isAccessibleFromDirection(enginetype::GridCell cellTo, Action directionToMove) {
    int element = Feld[cellTo.x][cellTo.y];
    return (element_info[element].access_direction & (getOppositeDirection(directionToMove)));
}


/**
 * Check if given gridcell contains element with IS_WALKABLE property.
 */
bool _isWalkable(const enginetype::GridCell cellTo, Action directionToMove) {
    int element = Feld[cellTo.x][cellTo.y];
    bool isAccessibleFromDirection = (directionToMove == Action::noop ? true : _isAccessibleFromDirection(cellTo, directionToMove));
    return IS_WALKABLE(element) && isAccessibleFromDirection;
}


/**
 * Checks if the direction the player wants to move in is walkable.
 * Walkable means that the player is able to stand in the cell which results
 * in the action being applied.
 */
bool isWalkable(const enginetype::GridCell cellFrom, Action directionToMove) {
    enginetype::GridCell cellTo = getCellFromAction(cellFrom, directionToMove);
    return inBounds(cellTo) && _isWalkable(cellTo, directionToMove);
}


/**
 * Checks if the GridCell cellTo is diggable.
 * For internal use, assumes bounds are validated elsewhere.
 */
bool _isDigable(const enginetype::GridCell cellTo) {
    int element = Feld[cellTo.x][cellTo.y];
    return IS_DIGGABLE(element);
}

/**
 * Checks if the direction the player wants to move in is diggable.
 * Diggable means that the player is able to stand in the cell which results
 * in the action being applied, after digging away the current element.
 */
bool isDigable(const enginetype::GridCell cellFrom, Action directionToMove) {
    enginetype::GridCell cellTo = getCellFromAction(cellFrom, directionToMove);
    return inBounds(cellTo) && _isDigable(cellTo);
}

/**
 * Checks if the GridCell cellTo is empty.
 * For internal use, assumes bounds are validated elsewhere.
 */
bool _isEmpty(const enginetype::GridCell cellTo) {
    return Feld[cellTo.x][cellTo.y] == EL_EMPTY;
}

/**
 * Checks if the direction the player wants to move in is empty.
 * in the action being applied, after digging away the current element.
 */
bool isEmpty(const enginetype::GridCell cellFrom, Action directionToMove) {
    enginetype::GridCell cellTo = getCellFromAction(cellFrom, directionToMove);
    return inBounds(cellTo) && _isEmpty(cellTo);
}


/**
 * Checks if the GridCell cellTo is a wall.
 * IS_WALL is a predefined macro for the default game wall types.
 * For internal use, assumes bounds are validated elsewhere.
 */
bool _isWall(const enginetype::GridCell cellTo) {
    int element = Feld[cellTo.x][cellTo.y];
    return IS_WALL(element);
}


/**
 * Checks if the direction the player wants to move in is a wall.
 * This works for predefined wall objects from the default game objects. A custom
 * non-passible object (which acts as a wall) won't be caught here.
 */
bool isWall(const enginetype::GridCell cellFrom, Action directionToMove) {
    enginetype::GridCell cellTo = getCellFromAction(cellFrom, directionToMove);
    return inBounds(cellTo) && _isWall(cellTo);
}


/**
 * Checks if the GridCell cellTo is a rock.
 * For internal use, assumes bounds are validated elsewhere.
 */
bool _isRock(const enginetype::GridCell cellTo) {
    int element = Feld[cellTo.x][cellTo.y];
    return element == EL_ROCK || element == EL_BD_ROCK;
}


/**
 * Checks if the direction the player wants to move in is a rock.
 * Rocks are either EL_ROCK or EL_BD_ROCK.
 */
bool isRock(const enginetype::GridCell cellFrom, Action directionToMove) {
    enginetype::GridCell cellTo = getCellFromAction(cellFrom, directionToMove);
    return inBounds(cellTo) && _isRock(cellTo);
}


/**
 * Checks if the GridCell cellTo is contains a collectible element.
 * For internal use, assumes bounds are validated elsewhere.
 */
bool _isCollectable(const enginetype::GridCell cellTo) {
    int element = Feld[cellTo.x][cellTo.y];
    return IS_COLLECTIBLE(element);
}

/**
 * Checks if resulting GridCell the player wants to move to contains a collectible element.
 */
bool isCollectable(const enginetype::GridCell cellFrom, Action directionToMove) {
    enginetype::GridCell cellTo = getCellFromAction(cellFrom, directionToMove);
    return inBounds(cellTo) && _isCollectable(cellTo);
}


/**
 * Checks if the GridCell cellPassed can be passed through from 
 * using the given action.
 * This mirrors an internal engine function logic. 
 * For internal use, assumes bounds are validated elsewhere.
 */
bool _canPassField(const enginetype::GridCell cellPassed, Action directionToMove) {
    int opposite_dir = MV_DIR_OPPOSITE(directionToMove);
    int dx = (directionToMove & MV_LEFT ? -1 : directionToMove & MV_RIGHT ? +1 : 0);
    int dy = (directionToMove & MV_UP   ? -1 : directionToMove & MV_DOWN  ? +1 : 0);
    int nextx = cellPassed.x + dx;
    int nexty = cellPassed.y + dy;
    int element = Feld[cellPassed.x][cellPassed.y];

    return ((IS_PASSABLE(element) && (element_info[element].access_direction &(opposite_dir))) &&
        !CAN_MOVE(element) &&
        IN_LEV_FIELD(nextx, nexty) && !IS_PLAYER(nextx, nexty) &&
        (IS_WALKABLE(Feld[nextx][nexty]) && (element_info[Feld[nextx][nexty]].access_direction &(directionToMove))) &&
        (level.can_pass_to_walkable || IS_FREE(nextx, nexty)));
}


/**
 * Checks if the GridCell cellPassed is passable
 * For internal use, assumes bounds are validated elsewhere.
 */
bool _isPassable(const enginetype::GridCell cellPassed, Action directionToMove) {
    int element = Feld[cellPassed.x][cellPassed.y];
    return IS_PASSABLE(element) && _canPassField(cellPassed, directionToMove);
}

/**
 * Checks if the direction the player wants to move in is passable.
 * Passable means that the player walks through the cell which results in the
 * action being applied, and into the next cell following the same direction.
 */
bool isPassable(const enginetype::GridCell cellFrom, Action directionToMove) {
    // Skip no action.
    if (directionToMove == Action::noop) {return true;}
    enginetype::GridCell cellPassed = getCellFromAction(cellFrom, directionToMove);
    return inBounds(cellPassed) && _isPassable(cellPassed, directionToMove);
}


/**
 * Checks if the action will move the player.
 * For internal use, assumes bounds are validated elsewhere.
 */
bool _isActionMoveable(const enginetype::GridCell cellTo, Action directionToMove) {
    return !_isWall(cellTo) && (_isWalkable(cellTo, directionToMove) || _isCollectable(cellTo) || _isPassable(cellTo, directionToMove) ||
         _isDigable(cellTo) || isGridEmpty(cellTo));
}


/**
 * Checks if the action will move the player.
 * Player can move if they are not walking into a wall, and the GridCell in the direction
 * the player wants to move is either walkable, passable, or contains a collectable item.
 */
bool isActionMoveable(const enginetype::GridCell cellFrom, Action directionToMove) {
    // Skip no action.
    if (directionToMove == Action::noop) {return true;}
    enginetype::GridCell cellTo = getCellFromAction(cellFrom, directionToMove);
    return inBounds(cellTo) && _isActionMoveable(cellTo, directionToMove);
}

} //namespace enginehelper