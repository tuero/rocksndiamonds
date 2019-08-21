
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
     * Get the controller type defined by user CLA
     */
    enginetype::ControllerType getControllerType();

    /*
     * Get the replay game name
     */
    std::string getReplayFileName();


// -------------------------------------------------------
// -------------------Level Information-------------------
// -------------------------------------------------------


    /*
     * Call engine functions to load the levelset
     */
    void setLevelSet();

    /*
     * Get the levelset used
     */
    std::string getLevelSet();

    /*
     * Call engine function to load the given level
     */
    void loadLevel(int level_num);

    /*
     * Get the level number from command line argument
     */
    int getLevelNumber();

    /*
     * Get the level height
     */
    int getLevelHeight();

    /*
     * Get the level width
     */
    int getLevelWidth();



// -------------------------------------------------------
// -----------------Map Item Information------------------
// -------------------------------------------------------


    /*
     * Get the item located at the given (x,y) grid location
     */
    int getGridItem(enginetype::GridCell cell);

    /*
     * Get the player grid position
     */
    enginetype::GridCell getPlayerPosition();

    /*
     * Get the current goal location (defined by distance of 0)
     */
    enginetype::GridCell getCurrentGoalLocation();

    /*
     * Returns true if a wall is on the direction the player wants to move
     * Assumes simulator is in the current state to check
     */
    bool isWall(Action action);

    /*
     * Used in PFA_MCTS
     * Checks if the player tree can expand to neighbouring gridcell given a list of allowed gridcells
     */
    bool canExpand(Action action, std::vector<enginetype::GridCell> &allowed_cells);

    /*
     * Check if two grid cells are neighbours
     */
    bool checkIfNeighbours(enginetype::GridCell left, enginetype::GridCell right);



// -------------------------------------------------------
// ---------------Custom Level Programming----------------
// -------------------------------------------------------


    /*
     * Count how many of a specified element in the game
     */
    int countNumOfElement(int element);

    /*
     * Add the specified element to the game
     */
    void spawnElement(int element, int dir, enginetype::GridCell gridCell);

    /*
     * Get all empty grid cells
     */
    void getEmptyGridCells(std::vector<enginetype::GridCell> &emptyGridCells);



// -------------------------------------------------------
// -----------------Game Engine State---------------------
// -------------------------------------------------------

    /*
     * Check if the current status of the engine is loss of life
     */
    bool engineGameFailed();

    /*
     * Check if the current status of the engine is level solved
     */
    bool engineGameSolved();

    /*
     * Set the action for the engine to perform on behalf of the player on
     * the next iteration
     *
     * @param action -> Action to perform (may be noop)
     */
    void setEnginePlayerAction(Action action);

    /*
     * Set the action for the engine to perform on behalf of the player on
     * the next iteration as a random action
     */
    void setEngineRandomPlayerAction();

    /*
     * Get the currently stored player action
     */
    int getEnginePlayerAction();

    /*
     * Simulate the engine ahead a single tick
     */
    void engineSimulateSingle();

    /*
     * Simulate the engine ahead
     * This performs ENGINE_RESOLUTION ticks
     */
    void engineSimulate();

    /*
     * Set flag for simulating
     * This will cause blocking actions in engine such as not rending to screen
     * Profiling shows a 10x in speed with simulator_flag set
     */
    void setSimulatorFlag(bool simulator_flag);

    /*
     * Get the simulator flag status
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
    int getGridDistanceToGoal(enginetype::GridCell grid_cell);

    /*
     * Get the player distance to the next abstract node
     * Internal abstract node distances are set by Dijsktra, used to help
     * player get around corners that fails by L1 shortest distance
     */
    int getPlayerDistanceToNextNode();

    /*
     * Find the grid location of the goal, given by enginetype::FIELD_GOAL
     */
    enginetype::GridCell findGoalLocation();

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