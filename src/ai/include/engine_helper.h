/**
 * @file: engine_helper.h
 *
 * @brief: Interface for controllers to access information about the engine state.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef ENGINE_HELPER_H
#define ENGINE_HELPER_H


// ------------- Includes  -------------

// Standard Libary/STL
#include <vector>
#include <deque>
#include <cstdint>

// Includes
#include "engine_types.h"


namespace enginehelper {

    static const uint64_t MAX_HASH = UINT64_MAX;
    static const int MAX_DIR = 32;


    /**
     * Get the controller type defined by user command line argument.
     * 
     * The type of available controllers are defined by system.h by enum controller_type,
     * which is set by the command line argument -controller <CONTROLLER_NAME>.
     *
     * @return Contoller enum type.
     */
    ControllerType getControllerType();

    /**
     * Get the replay file name.
     *
     * The replay file name is supplied by the command line argument -replay <REPLAY_FILE>.
     * 
     * @return Replay file name string.
     */
    std::string getReplayFileName();

    /**
     * Get the optional CLA for general use.
     * 
     * @return Integer optional CLA.
     */
    int getOptParam();


// -------------------------------------------------------
// ------------------Action Information-------------------
// -------------------------------------------------------

    /**
     * Get string representation of action.
     *
     * @param action The action enum type.
     * @return The string representation of the action.
     */
    std::string actionToString(const Action action);

    /**
     * Get the enum representation of the action.
     *
     * @param str The action string.
     * @return The action enum type.
     */
    Action stringToAction(const std::string &str);

    /**
     * Get the opposite direction.
     * 
     * @param action The reference action
     * @return The opposite action direction.
     */
    Action getOppositeDirection(Action action);


// -------------------------------------------------------
// -------------------Level Information-------------------
// -------------------------------------------------------


    /**
     * Call engine functions to automatically load the levelset.
     *
     * This is used when supplying the command line argument -levelset <LEVELSET>, so that
     * a specific levelset/level can be preloaded and started without having to manually 
     * select the levelset/level using the GUI.
     *
     * The engine function OpenAll() will attempt to load the levelset, regardless if it is 
     * valid. If the levelset doesn't exist, OpenAll() falls back to using a default
     * empty level.
     */
    void setLevelSet(bool openall=false);

    /**
     * Get the levelset currently set in the engine.
     *
     * @return The string name of the levelset.
     */
    const std::string getLevelSet();

    /**
     * Call engine function to load the given level.
     *
     * The struct options.level_number is referenced by the engine when loading the
     * level by LoadLevel(int).
     *
     * The engine function LoadLevel(int) will attempt to load the level, regardless if it is 
     * valid. If the level doesn't exist, LoadLevel(int) falls back to using a default
     * empty level.
     *
     * @param level_num The integer level number.
     */
    void loadLevel(int level_num);

    /**
     * Get the level number loaded in the level struct
     *
     * @return The level number
     */
    int getLevelNumber();

    /**
     * Restart the level.
     * Game is reinitialized using the current loaded level.
     */
    void restartLevel();

    /**
     * Get the level height of the level currently loaded in the engine.
     *
     * @return The level height as measured in grid tiles.
     */
    int getLevelHeight();

    /**
     * Get the level width of the level currently loaded in the engine.
     *
     * @return The level width as measured in grid tiles.
     */
    int getLevelWidth();

    /**
     * Get the number of gems needed to open the exit.
     * 
     * @return The number of gems.
     */
    int getLevelGemsNeeded();

    /**
     * Get the number of remaining gems needed to open the exit.
     * 
     * @return the count of gems remaining to open the exit.
     */
    int getLevelRemainingGemsNeeded();


// -------------------------------------------------------
// ---------------Grid Action Information-----------------
// -------------------------------------------------------

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


// -------------------------------------------------------
// -----------------Map Item Information------------------
// -------------------------------------------------------
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
     * Get the item element located at the given GridCell (x,y) location.
     *
     * @param The GridCell to locate.
     * @return The integer code for the object at the grid location, or empty if GridCell out of bounds.
     */
    int getGridElement(enginetype::GridCell cell);

    /**
     * Get the item MovPos at the given GridCell (x,y) location.
     *
     * @param The grid cell to locate.
     * @return The integer code for the object offset while moving.
     */
    int getGridMovPos(enginetype::GridCell cell);

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
    bool isGridEmpty(enginetype::GridCell cell);

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
    void spawnElement(int element, int dir, enginetype::GridCell gridCell);

    /**
     * Get the current goal location (defined by distance of 0).
     *
     * If no grid location has distance 0, then (-1, -1) is returned and the calling method
     * must deal with this.
     *
     * @return The GridCell struct containing the goal.
     */
    enginetype::GridCell getCurrentGoalLocation();


    // -------------------------------------------------------
    // -------------Element Property Information--------------
    // -------------------------------------------------------


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
     * Checks if the direction the player wants to move in is walkable.
     * Walkable means that the player is able to stand in the cell which results
     * in the action being applied.
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is walkable.
     */
    bool isWalkable(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

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
     * Checks if resulting GridCell the player wants to move to contains a collectible element.
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
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is passable.
     */
    bool isPassable(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);

    /**
     * Checks if the action will move the player.
     * Player can move if they are not walking into a wall, and the GridCell in the direction
     * the player wants to move is either walkable, passable, or contains a collectable item.
     *
     * Assumes the current state to check is already set in the engine if no player cell
     * is given as the second argument.
     *
     * @param cellFrom The cell the player is in.
     * @param directionToMove The action the player wants to attempt to make.
     * @return True if the action the player wants to take is passable.
     */
    bool isActionMoveable(const enginetype::GridCell cellFrom, Action directionToMove = Action::noop);



// -------------------------------------------------------
// ------------------Engine Status------------------------
// -------------------------------------------------------

    /**
     * Check if the current status of the engine is loss of life.
     * The internal engine checkGameFailed() function is called. 
     *
     * @return True if the current status of the engine is failed.
     */
    bool engineGameFailed();

    /**
     * Check if the current status of the engine is level solved.
     * Structs game.LevelSolved and game.LevelSolved_GameEnd need to be checked.
     *
     * @return True if the current status of the engine is solved.
     */
    bool engineGameSolved();

    /**
     * Check if the current status of the engine is failed or solved
     *
     * @return True if the current status of engine is failed or solved.
     */
    bool engineGameOver();

    /**
     * Set the game engine mode status to GAME_MODE_QUIT.
     * This will trigger the program to end after cleanup. 
     */
    void setEngineGameStatusModeQuit();

    /**
     * Set the game engine mode status to GAME_MODE_PLAYING.
     * This will allow the game loop to query actions from controller. 
     */
    void setEngineGameStatusModePlaying();

    /**
     * Check if the player is currently in the middle of executing an action.
     * 
     * @return True if the player is in the middle of grid cells (moving)
     */
    bool isPlayerDoneAction();

    /**
     * Set the action for the engine to perform on behalf of the player on the next iteration.
     *
     * The default agent (in single player mode) is in the struct stored_player[0]. The engine
     * looks at the action member each tick and performs the stored action.
     *
     * @param action The action to perform (may be noop)
     */
    void setEnginePlayerAction(Action action);

    /**
     * Set the stored player's action as a valid random action.
     *
     * To allow for fast simulations, this doesn't check whether the action is
     * useful i.e. not moving left if there is a wall to the left of the player.
     */
    void setEngineRandomPlayerAction();

    /**
     * Get the currently stored player action.
     *
     * @return The integer representation of the action to perform.
     */
    int getEnginePlayerAction();

    /**
     * Get the current score of the game.
     *
     * @return The score of the game.
     */
    int getCurrentScore();

    /**
     * Get the Time left in the game.
     */
    int getTimeLeftScore();

    /**
     * Simulate the engine ahead a single tick.
     *
     * This calls the HandleGameActions() engine function, which performs one game tick.
     * The players action needs to be set before calling this function. If the player is
     * currently in motion, its action is continued.
     */
    void engineSimulateSingle();

    /**
     * Simulate the engine ahead ENGINE_RESOLUTION=8 ticks.
     *
     * At normal speed, objects take 8 game ticks to take an action (move completely to a 
     * neighbouring grid cell). Objects take multiples or fractions of ENGINE_RESOLUTION
     * if they have a faster/slower movement speed, but the player always takes 
     * ENGINE_RESOLUTION to perform and complete an action.
     *
     * This calls the HandleGameActions() engine function ENGINE_RESOLUTION times. 
     * The players action needs to be set before calling this function. 
     */
    void engineSimulate();

    /**
     * Set flag for simulating.
     *
     * This should be set to True whenever the controller wants to simulate, and 
     * set back to false before returning the action back to the calling function.
     * By default, this will be returned back to false by the calling function.
     * 
     * This will cause blocking actions in engine such as not rending to screen.
     * Profiling shows a 10x in speed with simulator_flag set, which is important when
     * wanting to do many simulations such as in MCTS.
     *
     * @param simulator_flag
     */
    void setSimulatorFlag(bool simulatorFlag);

    /**
     * Get the simulator flag status.
     *
     * @param True if the simulator flag is currently set.
     */
    bool isSimulating();


// -------------------------------------------------------
// -------------------State Hashing-----------------------
// -------------------------------------------------------

    /**
     * Initialize Zorbrist tables, used to hash game board states
     */
    void initZorbristTables();

    /**
     * Get the hash representation of a vector grid cell path.
     */
    uint64_t gridcellPathToHash(const std::deque<enginetype::GridCell> &path);

    /**
     * Get the hash representation of the current state in the engine
     */
    uint64_t stateToHash();


// -------------------------------------------------------
// ----------------Distance Functions---------------------
// -------------------------------------------------------

    /**
     * Get L1 distance between two gridcells
     */
    int getL1Distance(enginetype::GridCell left, enginetype::GridCell right);

    /**
     * Get the distance to goal (defined as distances[x][y] = 0) from given GridCell
     * This is set by distance metric, usually L1.
     * 
     * @param cell THe GridCell to get the distance for.
     */
    int getGridDistanceToGoal(const enginetype::GridCell cell);

    /**
     * Set the distance to goal manually for more exotic distance functions.
     * 
     * @param cell The GridCell to set the distance for.
     * @param value The distance to set.
     */
    void setGridDistanceToGoal(const enginetype::GridCell cell, short value);

    /**
     * Find the grid location of the goal, given by a distance measure of 0.
     * If no distance value of 0 exists, {-1,-1} is returned.
     * 
     * @return The gridcell which has current distance value of 0.
     */
    enginetype::GridCell findDistanceGoalCell();

    /**
     * Find the grid location of the exit, given by EL_EXIT_OPEN.
     * 
     * @return The GridCell of the EL_EXIT_OPEN element, or {-1, -1} if DNE.
     */
    enginetype::GridCell findExitLocation();

    /**
     * Reset the board distances.
     * The reset value is -1, as 0 indicates the goal gridcell.
     */
    void resetBoardDistances();

    /**
     * Set the grid distances to goal using L1 distance
     * If no goal cell is given, will attempt to use the open exit location
     * given by EL_EXIT_OPEN, it it exists.
     * 
     * @param goalCell The goal cell 
     */
    void setBoardDistancesL1(const enginetype::GridCell goalCell = {-1, -1});



    // ------------- flattened range accessors  -------------
    // See: https://stackoverflow.com/questions/26948099/stdcopy-for-multidimensional-arrays
    #define AUTO_RETURN(...) noexcept(noexcept(__VA_ARGS__)) -> decltype(__VA_ARGS__) {return (__VA_ARGS__);}

    template <typename T>
    constexpr auto decayed_begin(T&& c)
    AUTO_RETURN(std::begin(std::forward<T>(c)))

    template <typename T>
    constexpr auto decayed_end(T&& c)
    AUTO_RETURN(std::end(std::forward<T>(c)))

    template <typename T, std::size_t N>
    constexpr auto decayed_begin(T(&c)[N])
    AUTO_RETURN(reinterpret_cast<typename std::remove_all_extents<T>::type*>(c))

    template <typename T, std::size_t N>
    constexpr auto decayed_end(T(&c)[N])
    AUTO_RETURN(reinterpret_cast<typename std::remove_all_extents<T>::type*>(c + N))
}


#endif  //ENGINEHELPER_H