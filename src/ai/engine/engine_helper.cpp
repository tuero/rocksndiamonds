/**
 * @file: engine_helper.cpp
 *
 * @brief: Interface for controllers to access information about the engine state.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "engine_helper.h"

#include <iostream>
#include <array>
#include <algorithm>
#include <map>
#include <random>

// RNG
#include "../util/rng.h"

//Logging
#include "../util/logger.h"
#include <plog/Log.h>


extern int spriteIDs[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int spriteIDCounter;

namespace enginehelper {

// Hashing data structures
uint64_t zobristElement[MAX_LEV_FIELDX * MAX_LEV_FIELDY][MAX_NUM_ELEMENTS];


/**
 * Get the controller type defined by user command line argument.
 */
ControllerType getControllerType() {
    return options.controller_type;
}

/**
 * Get the replay file name.
 */
std::string getReplayFileName() {
    std::string replay_file(options.replay_file);
    return replay_file;
}


/**
 * Get the option CLA for general use.
 */
int getOptParam() {
    return options.opt;
}


// -------------------------------------------------------
// ------------------Action Information-------------------
// -------------------------------------------------------

/**
 * Fast lookup action descriptions.
 */
static const std::unordered_map<Action, std::string> ACTION_TO_STRING = 
{
    {Action::right, "right"},
    {Action::down,  "down"},
    {Action::left,  "left"},
    {Action::up,    "up"},
    {Action::noop,  "noop"}
};
static const std::unordered_map<std::string, Action> STRING_TO_ACTION = 
{
    {"right",   Action::right},
    {"down",    Action::down},
    {"left",    Action::left},
    {"up",      Action::up},
    {"noop",    Action::noop}
};

/**
 * Get string representation of action.
 */
std::string actionToString(const Action action) {
    auto it = ACTION_TO_STRING.find(action);
    return (it == ACTION_TO_STRING.end() ? ACTION_TO_STRING.find(Action::noop)->second : it->second);
}


/**
 * Get the enum representation of the action.
 */
Action stringToAction(const std::string &str) {
    auto it = STRING_TO_ACTION.find(str);
    return (it == STRING_TO_ACTION.end() ? Action::noop : it->second);
}


/**
 * Get the opposite direction.
 */
Action getOppositeDirection(Action action) {
    return static_cast<Action>(MV_DIR_OPPOSITE(action));
}


// -------------------------------------------------------
// -------------------Level Information-------------------
// -------------------------------------------------------


/**
 * Call engine functions to automatically load the levelset.
 */
void setLevelSet() {
    // Need to load levelset before openall
    // Maybe get setLevelset to check if replay, then get the levelset from there?
    // Kind of messy, need a better

    // No levelset given
    if (options.level_set == NULL) {return;}

    std::string level_set(options.level_set);

    try{
        // Initialize leveldir_current and related objects
        LoadLevelInfo();
        PLOGI_(logger::FileLogger) << "Setting levelset: \"" << level_set << "\"";
        PLOGI_(logger::ConsoleLogger) << "Setting levelset: \"" << level_set << "\"";

        // Set levelset to save
        leveldir_current->fullpath = options.level_set;
        leveldir_current->subdir = options.level_set;
        leveldir_current->identifier = options.level_set;
        leveldir_current->in_user_dir = false;

        // Save the levelset
        // We save because on startup, the previously saved levelset is loaded
        SaveLevelSetup_LastSeries();
        OpenAll(); 
    }
    catch (...){
        PLOGE_(logger::FileLogger) << "Something went wrong trying to load levelset " << level_set;
        PLOGE_(logger::ConsoleLogger) << "Something went wrong trying to load levelset " << level_set;
        CloseAllAndExit(1);
    }
}

/**
 * Get the levelset currently set in the engine.
 */
const std::string getLevelSet() {
    return leveldir_current->subdir;
}


/**
 * Call engine function to load the given level.
 */
void loadLevel(int level_num) {
    options.level_number = level_num;
    PLOGI_(logger::FileLogger) << "Loading level: " << level_num;
    PLOGI_(logger::ConsoleLogger) << "Loading level: " << level_num;
    LoadLevel(level_num);
}


/**
 * Get the level number loaded in the level struct
 */
int getLevelNumber() {
    return level.file_info.nr;
}


/**
 * Restart the level.
 * Game is reinitialized using the current loaded level.
 */
void restartLevel() {
    PLOGI_(logger::FileLogger) << "Restarting level: " << getLevelNumber();
    PLOGI_(logger::ConsoleLogger) << "Restarting level: " << getLevelNumber();
    
    InitGame();
}


/**
 * Get the level height of the level currently loaded in the engine.
 */
int getLevelHeight() {
    return level.fieldy;
}


/**
 * Get the level width of the level currently loaded in the engine.
 */
int getLevelWidth() {
    return level.fieldx;
}


/**
 * Get the number of gems needed to open the exit.
 */
int getLevelGemsNeeded() {
    return level.gems_needed;
}


/**
 * Get the number of remaining gems needed to open the exit.
 */
int getLevelRemainingGemsNeeded() {
    return game.gems_still_needed;
}


// -------------------------------------------------------
// ---------------Grid Action Information-----------------
// -------------------------------------------------------

/**
 * Check if two grid cells are neighbours, defined by being separated by Euclidean distance of 1.
 */
bool checkIfNeighbours(enginetype::GridCell left, enginetype::GridCell right) {
    return std::abs(left.x - right.x) + std::abs(left.y - right.y) == 1;
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
Action getActionFromNeighbours(enginetype::GridCell from, enginetype::GridCell to) {
    if (!checkIfNeighbours(from, to) && !(from == to)) {
        return Action::noop;
    }
    return DELTA_ACTION_MAP.at({to.x - from.x, to.y - from.y});
}


/**
 * Get the resulting cell from applying the action in the given cell.
 */
enginetype::GridCell getCellFromAction(Action action, const enginetype::GridCell from) {
    std::array<int, 2> delta = ACTION_DELTA_MAP.at(action);
    return {from.x + delta[0], from.y + delta[1]};
}


// -------------------------------------------------------
// -----------------Map Item Information------------------
// -------------------------------------------------------


/**
 * Convert a gridcell to a flat index
 */
int cellToIndex(const enginetype::GridCell &cell) {
    return (cell.y * getLevelWidth() + cell.x);
}

/**
 * Checks if the given gridcell is in bounds of the level field.
 */
bool inBounds(const enginetype::GridCell &cell) {
    return (cell.x >= 0 && cell.x < level.fieldx && cell.y >= 0 && cell.y < level.fieldy);
}


/**
 * Check if the element in the gridcell is a temporary element.
 * For internal use when bound-check is implicitly done.
 */
bool _isTemporaryElement(const enginetype::GridCell &cell) {
    return Feld[cell.x][cell.y] >= NUM_DRAWABLE_ELEMENTS;
}


/**
 * Check if the element in the gridcell is a temporary element.
 */
bool isTemporaryElement(const enginetype::GridCell &cell) {
    return inBounds(cell) && _isTemporaryElement(cell);
}


/**
 * Checks if a given cell contains a valid sprite.
 * For internal use when bound-check is implicitly done.
 */
bool _isValidSprite(const enginetype::GridCell &cell) {
    int element = Feld[cell.x][cell.y];
    return (element != EL_EMPTY && element != EL_SAND) && !_isTemporaryElement(cell);
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
    if (spriteID == -1) {return true;}
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
 * Check if the player is currently in the middle of executing an action.
 */
bool isPlayerDoneAction() {
    return stored_player[0].MovPos == 0;
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


// -------------------------------------------------------
// -------------Element Property Information--------------
// -------------------------------------------------------


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
 * Check if the item in the gridcell is moving.
 * 
 * Moving is defined as not being completely inside a single grid cell
 * i.e. it is moving between gridcells. This means that if an item is free
 * falling between multiple cells, there will be a single game step where is 
 * check is false.
 */
bool isMoving(const enginetype::GridCell cell) {
    return inBounds(cell) && MovPos[cell.x][cell.y] != 0;
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
 * Internal check (doesn't valid bounds), check if player can perform action 
 * and walk into cell cellTo.
 */
bool _isWalkable(Action action, const enginetype::GridCell cellTo) {
    int element = Feld[cellTo.x][cellTo.y];
    bool isAccessibleFromDirection = (element_info[element].access_direction &(getOppositeDirection(action)));
    return IS_WALKABLE(element) && isAccessibleFromDirection;
}

/**
 * Checks if the direction the player wants to move in is walkable.
 * Walkable means that the player is able to stand in the cell which results
 * in the action being applied.
 */
bool isWalkable(Action action, const enginetype::GridCell cellFrom) {
    // Skipp no action.
    if (action == Action::noop) {return true;}

    enginetype::GridCell cellTo = getCellFromAction(action, 
        (cellFrom.x == -1 || cellFrom.y == -1) ? getPlayerPosition() : cellFrom);

    return inBounds(cellTo) && _isWalkable(action, cellTo);
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
 * in the action being applied.
 */
bool isDigable(Action action, const enginetype::GridCell cellFrom) {
    enginetype::GridCell cellTo = getCellFromAction(action, 
        (cellFrom.x == -1 || cellFrom.y == -1) ? getPlayerPosition() : cellFrom);

    return inBounds(cellTo) && _isDigable(cellTo);
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
bool isWall(Action action, const enginetype::GridCell cellFrom) {
    enginetype::GridCell cellTo = getCellFromAction(action, 
        (cellFrom.x == -1 || cellFrom.y == -1) ? getPlayerPosition() : cellFrom);

    return inBounds(cellTo) && _isWall(cellTo);
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
bool isCollectable(Action action, const enginetype::GridCell cellFrom) {
    enginetype::GridCell cellTo = getCellFromAction(action, 
        (cellFrom.x == -1 || cellFrom.y == -1) ? getPlayerPosition() : cellFrom);

    return inBounds(cellTo) && _isCollectable(cellTo);
}


/**
 * Checks if the GridCell cellPassed can be passed through from 
 * using the given action.
 * This mirrors an internal engine function logic. 
 * For internal use, assumes bounds are validated elsewhere.
 */
bool _canPassField(const enginetype::GridCell cellPassed, Action action) {
    int opposite_dir = MV_DIR_OPPOSITE(action);
    int dx = (action & MV_LEFT ? -1 : action & MV_RIGHT ? +1 : 0);
    int dy = (action & MV_UP   ? -1 : action & MV_DOWN  ? +1 : 0);
    int nextx = cellPassed.x + dx;
    int nexty = cellPassed.y + dy;
    int element = Feld[cellPassed.x][cellPassed.y];

    return ((IS_PASSABLE(element) && (element_info[element].access_direction &(opposite_dir))) &&
        !CAN_MOVE(element) &&
        IN_LEV_FIELD(nextx, nexty) && !IS_PLAYER(nextx, nexty) &&
        (IS_WALKABLE(Feld[nextx][nexty]) && (element_info[Feld[nextx][nexty]].access_direction &(action))) &&
        (level.can_pass_to_walkable || IS_FREE(nextx, nexty)));
}


/**
 * Checks if the GridCell cellPassed is passable
 * For internal use, assumes bounds are validated elsewhere.
 */
bool _isPassable(Action action, const enginetype::GridCell cellPassed) {
    int element = Feld[cellPassed.x][cellPassed.y];
    return IS_PASSABLE(element) && _canPassField(cellPassed, action);
}

/**
 * Checks if the direction the player wants to move in is passable.
 * Passable means that the player walks through the cell which results in the
 * action being applied, and into the next cell.
 */
bool isPassable(Action action, const enginetype::GridCell cellFrom) {
    // Skip no action.
    if (action == Action::noop) {return true;}

    enginetype::GridCell cellPassed = getCellFromAction(action, 
        (cellFrom.x == -1 || cellFrom.y == -1) ? getPlayerPosition() : cellFrom);

    return inBounds(cellPassed) && _isPassable(action, cellPassed);
}


/**
 * Checks if the action will move the player.
 * For internal use, assumes bounds are validated elsewhere.
 */
bool _isActionMoveable(Action action, const enginetype::GridCell cellTo) {
    return !_isWall(cellTo) && (_isWalkable(action, cellTo) || _isCollectable(cellTo) || _isPassable(action, cellTo) ||
         _isDigable(cellTo) || isGridEmpty(cellTo));
}


/**
 * Checks if the action will move the player.
 * Player can move if they are not walking into a wall, and the GridCell in the direction
 * the player wants to move is either walkable, passable, or contains a collectable item.
 */
bool isActionMoveable(Action action, const enginetype::GridCell cellFrom) {
    // Skip no action.
    if (action == Action::noop) {return true;}

    enginetype::GridCell cellTo = getCellFromAction(action, 
        (cellFrom.x == -1 || cellFrom.y == -1) ? getPlayerPosition() : cellFrom);

    return inBounds(cellTo) && _isActionMoveable(action, cellTo);
}


// -------------------------------------------------------
// ---------------Custom Level Programming----------------
// -------------------------------------------------------

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


// -------------------------------------------------------
// -----------------Game Engine State---------------------
// -------------------------------------------------------


/**
 * Check if the current status of the engine is loss of life.
 */
bool engineGameFailed() {
    return checkGameFailed();
}


/**
 * Check if the current status of the engine is level solved.
 */
bool engineGameSolved() {
    return (game.LevelSolved && !game.LevelSolved_GameEnd);
}


/**
 * Check if the current status of the engine is failed or solved
 */
bool engineGameOver() {
    return engineGameFailed() || engineGameSolved();
}


/**
 * Set the game engine mode status to GAME_MODE_QUIT.
 * This will trigger the program to end after cleanup. 
 */
void setEngineGameStatusModeQuit() {
    game_status = GAME_MODE_QUIT;
}

/**
 * Set the game engine mode status to GAME_MODE_PLAYING.
 * This will allow the game loop to query actions from controller. 
 */
void setEngineGameStatusModePlaying() {
    game_status = GAME_MODE_PLAYING;
}


/**
 * Set the action for the engine to perform on behalf of the player on the next iteration.
 */
void setEnginePlayerAction(Action action) {
    stored_player[0].action = action;
}


/**
 * Set the stored player's action as a valid random action.
 */
void setEngineRandomPlayerAction() {
    static std::random_device rand_dev;
    static std::mt19937 gen(rand_dev());
    static std::uniform_int_distribution<int> dist(0, NUM_ACTIONS-1);

    int action = dist(gen);
    stored_player[0].action = (action == NUM_ACTIONS-1) ? 0 : (1 << action);
}


/**
 * Get the currently stored player action.
 */
int getEnginePlayerAction() {
    return stored_player[0].action;
}


/**
 * Get the current score of the game.
 */
int getCurrentScore() {
    return game.score;
}


/**
 * Get the Time left in the game.
 */
int getTimeLeftScore() {
    return TimeLeft;
}


/**
 * Simulate the engine ahead a single tick.
 */
void engineSimulateSingle() {
    HandleGameActions();
}


/**
 * Simulate the engine ahead ENGINE_RESOLUTION game ticks.
 */
void engineSimulate() {
    for (int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        HandleGameActions();
    }
}

/**
 * Set flag for simulating.
 */
void setSimulatorFlag(bool simulatorFlag) {
    is_simulating = simulatorFlag;
}


/**
 * Get the simulator flag status.
 */
bool isSimulating() {
    return is_simulating;
}


// -------------------------------------------------------
// -------------------State Hashing-----------------------
// -------------------------------------------------------


/**
 * Initialize Zorbrist tables, used to hash game board states
 */
void initZorbristTables() {
    for (int i = 0; i < MAX_LEV_FIELDX*MAX_LEV_FIELDY; i++) {
        // K is item type
        for (int k = 0; k < MAX_NUM_ELEMENTS; k++) {
            zobristElement[i][k] = RNG::getRandomNumber();
        }
    }
}


/**
 * Get the hash representation of a vector grid cell path.
 */
uint64_t gridcellPathToHash(const std::deque<enginetype::GridCell> &path) {
    uint64_t hashValue = 0; 
    for (auto const & cell : path) {
        hashValue ^= zobristElement[cellToIndex(cell)][EL_EMPTY];
    }

    return hashValue;
}


/**
 * Get the hash representation of the current state in the engine
 */
uint64_t stateToHash() {
    int px = stored_player[0].jx;
    int py = stored_player[0].jy;
    uint64_t hashValue = 0; 

    // Set initial hash
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            hashValue ^= zobristElement[y*level.fieldx + x][Feld[x][y]];
        }
    }

    hashValue ^= zobristElement[py*level.fieldx + px][80];

    return hashValue;
}


// -------------------------------------------------------
// ----------------Distance Functions---------------------
// -------------------------------------------------------


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
int getGridDistanceToGoal(const enginetype::GridCell goalCell) {
    if (!inBounds(goalCell)) {
        return -1;
    }
    return distances[goalCell.x][goalCell.y];
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
 * Find the grid location of the goal, given by EL_EXIT_OPEN
 */
enginetype::GridCell findExitLocation() {
    enginetype::GridCell goal_cell = {-1, -1};
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (Feld[x][y] == EL_EXIT_OPEN) {
                goal_cell.x = x;
                goal_cell.y = y;
                return goal_cell;
            }
        }
    }
    return goal_cell;
}


/**
 * Set the grid distances to goal using L1 distance
 */
void setBoardDistancesL1(const enginetype::GridCell goalCell) {
    // PLOGI_(logger::FileLogger) << "Setting board distances.";

    // Initialize distances
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            distances[x][y] = -1;
        }
    }

    // Goal cell not explicitly given, so try to find the exit location.
    enginetype::GridCell goalCell_ = (inBounds(goalCell)) ? goalCell : findExitLocation();
    if (!inBounds(goalCell_)) {
        // PLOGI_(logger::FileLogger) << "Can't determine goal.";
        return;
    }

    // Set goal distance
    distances[goalCell_.x][goalCell_.y] = 0;

    // Set other grid distances
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            if (!_isWall({x, y})) {
                distances[x][y] = std::abs(goalCell_.x - x) + std::abs(goalCell_.y - y);
            }
        }
    }
}

}