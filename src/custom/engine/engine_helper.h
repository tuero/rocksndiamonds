
#ifndef ENGINE_HELPER_H
#define ENGINE_HELPER_H


// ------------- Includes  -------------
#include <iostream>
#include <chrono>
#include <array>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstdint>

#include "engine_types.h"
#include "action.h"
#include "../util/rng.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h> 

extern "C" {
    #include "../../main.h"
    #include "../../files.h"
}


namespace enginehelper {

    extern short distances[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    extern short max_distance;

    static const uint64_t MAX_HASH = UINT64_MAX;
    static const int MAX_DIR = 32;


    /*
     * Get the controller type defined by user CLA
     */
    enginetype::ControllerType getControllerType();

    /*
     * Call engine function to load the given level
     */
    void loadLevel(int level_num);

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
     * Returns true if a wall is on the direction the player wants to move
     * Assumes simulator is in the current state to check
     */
    bool isWall(Action action);

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

    /*
     * Initialize Zorbrist tables, used to hash game board states
     */
    void initZorbristTables();

    /*
     * Get the hash representation of the current state in the engine
     */
    uint64_t stateToHash();

    /*
     * Get the replay game name
     */
    std::string getReplayFileName();

    /*
     * Get the players current shortest path distance to goal
     * This uses distance tile maps pre-calculated using Dijkstra algorithm,
     * NOT Euclidean distance.
     */
    float getDistanceToGoal();


    void dijkstra();



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