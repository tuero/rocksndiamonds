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
#include <iostream>
#include <chrono>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>
#include <cstdint>
#include <cstdlib>              // abs

#include "engine_types.h"
#include "action.h"
#include "../util/rng.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h> 

extern "C" {
    #include "../../main.h"
    #include "../../files.h"
    #include "../../init.h"
}


namespace enginehelper {

    extern short distances[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    extern short max_distance;

    static std::map<enginetype::GridCell, std::vector<enginetype::GridCell>> grid_neighbours_;

    static const uint64_t MAX_HASH = UINT64_MAX;
    static const int MAX_DIR = 32;


    /*
     * Get the controller type defined by user command line argument.
     * 
     * The type of available controllers are defined by system.h by enum controller_type,
     * which is set by the command line argument -controller <CONTROLLER_NAME>.
     *
     * @return Contoller enum type.
     */
    enginetype::ControllerType getControllerType();

    /*
     * Get the replay file name.
     *
     * The replay file name is supplied by the command line argument -replay <REPLAY_FILE>.
     * 
     * @return Replay file name string.
     */
    std::string getReplayFileName();


// -------------------------------------------------------
// -------------------Level Information-------------------
// -------------------------------------------------------


    /*
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
    void setLevelSet();

    /*
     * Get the levelset currently set in the engine.
     *
     * @return The string name of the levelset.
     */
    std::string getLevelSet();

    /*
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

    /*
     * Get the level number from command line argument.
     *
     * @return The level number as set in the command line argument.
     */
    int getLevelNumber();

    /*
     * Get the level height of the level currently loaded in the engine.
     *
     * @return The level height as measured in grid tiles.
     */
    int getLevelHeight();

    /*
     * Get the level width of the level currently loaded in the engine.
     *
     * @return The level width as measured in grid tiles.
     */
    int getLevelWidth();



// -------------------------------------------------------
// -----------------Map Item Information------------------
// -------------------------------------------------------


    /*
     * Get the item located at the given GridCell (x,y) location.
     *
     * @param The grid cell to locate.
     * @return The integer code for the object at the grid location, or empty if gridcell out of bounds.
     */
    int getGridItem(enginetype::GridCell cell);

    /*
     * Get the grid cell that the player is currently located in.
     *
     * @return The GridCell struct containing the player.
     */
    enginetype::GridCell getPlayerPosition();

    /*
     * Get the current goal location (defined by distance of 0).
     *
     * If no grid location has distance 0, then (-1, -1) is returned and the calling method
     * must deal with this.
     *
     * @return The GridCell struct containing the goal.
     */
    enginetype::GridCell getCurrentGoalLocation();

    /*
     * Check if the grid cell at location (x,y) is empty.
     *
     * This only checks if the cell is designated by the empty integer ID, not whether
     * the grid cell is also dirt. A cell is empty if no object is currently in that
     * cell. Since it takes multiple ticks for objects to move betwene cells, there is 
     * also a temporary ID used for objects to claim GirdCells they are on route to travel
     * next to. 
     * 
     * @param cell The GridCell struct to check if empty.
     * @return True if the GirdCell is designated by the empty int code.
     */
    bool isGridEmpty(enginetype::GridCell cell);

    /*
     * Checks if the direction the player wants to move in is a wall.
     *
     * Assumes the current state to check is already set in the engine.
     *
     * @param action The action the player wants to attempt to make
     * @return True if the action the player wants to take is blocked by a wall.
     */
    bool isWall(Action action);

    /*
     * Checks if action is valid given restricted GridCells player is allowed in.
     *
     * Used in PFA_MCTS during the tree search. The allowed_cells is an abstraction of the
     * next abstract node to travel. This reduces the search tree for faster searches. 
     *
     * @param action The action the player wants to perform.
     * @param allowedCells A vector of cells which the player is restricted too.
     * @return True if the player the action want to perform stays in an allowed cell.
     */
    bool canExpand(Action action, std::vector<enginetype::GridCell> &allowed_cells);

    /*
     * Check if two grid cells are neighbours, defined by being separated by Euclidean distance of 1.
     *
     * @param left First GridCell struct to compare.
     * @param left Second GridCell struct to compare.
     * @return True if the two GridCells are neighbours.
     */
    bool checkIfNeighbours(enginetype::GridCell left, enginetype::GridCell right);



// -------------------------------------------------------
// ---------------Custom Level Programming----------------
// -------------------------------------------------------


    /*
     * Count how many of a specified element in the game.
     *
     * @param element The integer ID of the element to count.
     * @return The count of the element.
     */
    int countNumOfElement(int element);

    /*
     * Add the specified element to the level.
     *
     * @param element The element integer ID to spawn.
     * @param dir The movement direciton ID the object should start moving in.
     * @param gridCell The grid cell the element should spawn in.
     */
    void spawnElement(int element, int dir, enginetype::GridCell gridCell);

    /*
     * Get all grid cells which are empty.
     *
     * @return A vector of GridCells structs which do not have an object in it.
     */
    std::vector<enginetype::GridCell> getEmptyGridCells();



// -------------------------------------------------------
// -----------------Game Engine State---------------------
// -------------------------------------------------------

    /*
     * Check if the current status of the engine is loss of life.
     *
     * The internal engine checkGameFailed() function is called. 
     */
    bool engineGameFailed();

    /*
     * Check if the current status of the engine is level solved.
     *
     * The internal engine checkGameSolved() function is called.
     */
    bool engineGameSolved();

    /*
     * Set the action for the engine to perform on behalf of the player on the next iteration.
     *
     * The default agent (in single player mode) is in the struct stored_player[0]. The engine
     * looks at the action member each tick and performs the stored action.
     *
     * @param action The action to perform (may be noop)
     */
    void setEnginePlayerAction(Action action);

    /*
     * Set the stored player's action as a valid random action.
     *
     * This is a pseudo-smart random, in the sense that it will not attempt to move into
     * a wall if the player is currently blocked by said wall. This is 
     */
    void setEngineRandomPlayerAction();

    /*
     * Get the currently stored player action.
     *
     * @return The integer representation of the action to perform.
     */
    int getEnginePlayerAction();

    /*
     * Get the current score of the game.
     *
     * @return The score of the game.
     */
    int getCurrentScore();

    /*
     * Simulate the engine ahead a single tick.
     *
     * This calls the HandleGameActions() engine function, which performs one game tick.
     * The players action needs to be set before calling this function. If the player is
     * currently in motion, its action is continued.
     */
    void engineSimulateSingle();

    /*
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

    /*
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

    /*
     * Get the simulator flag status.
     *
     * @param True if the simulator flag is currently set.
     */
    bool isSimulating();


// -------------------------------------------------------
// -------------------State Hashing-----------------------
// -------------------------------------------------------

    /*
     * Initialize Zorbrist tables, used to hash game board states
     */
    void initZorbristTables();

    /*
     * Get the hash representation of the current state in the engine
     */
    uint64_t stateToHash();


// -------------------------------------------------------
// ----------------Distance Functions---------------------
// -------------------------------------------------------

    /*
     * Get L1 distance between two gridcells
     */
    int getL1Distance(enginetype::GridCell left, enginetype::GridCell right);

    /*
     * Get the minimum player distance in reference to given list of gridcells
     */
    int minDistanceToAllowedCells(std::vector<enginetype::GridCell> &goal_cells);

    /*
     * Get the players current shortest path distance to goal
     * This uses distance tile maps pre-calculated using Dijkstra's algorithm,
     * NOT Euclidean distance.
     */
    float getPlayerDistanceToGoal();

    /*
     * Get the distance to goal from given gridcell
     * This is set by distance metric, usually L1
     */
    int getGridDistanceToGoal(enginetype::GridCell gridCell);

    /*
     * Get the player distance to the next abstract node
     * Internal abstract node distances are set by Dijsktra, used to help
     * player get around corners that fails by L1 shortest distance
     */
    int getPlayerDistanceToNextNode();

    /*
     * Find the grid location of the exit, given by enginetype::FIELD_EXIT
     */
    enginetype::GridCell findExitLocation();

    /*
     * Set the grid distances to goal using Dijkstra's algorithm (shortest path)
     */
    void setBoardDistancesDijkstra(enginetype::GridCell goal_cell);

    /*
     * Set the grid distances to goal using L1 distance
     */
    void setBoardDistancesL1(enginetype::GridCell goal_cell);

    /*
     * Used in PFA_MCST
     * Set the internal grid cell distances to goal in abstract node
     * This helps MCTS get around corners that fails with just L1
     */
    void setAbstractNodeDistances(std::vector<enginetype::GridCell> goal_cells,
    std::vector<enginetype::GridCell> allowed_cells);



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