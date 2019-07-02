
#ifndef ENGINE_HELPER_H
#define ENGINE_HELPER_H


// ------------- Includes  -------------
#include <iostream>
#include <chrono>
#include <array>

#include "engine_types.h"
#include "action.h"

extern "C" {
    #include "../../main.h"
}


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
 * Print current engine status
 */
void debug_print();


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


#endif  //ENGINEHELPER_H