/**
 * @file: engine_types.cpp
 *
 * @brief: Readable typedefs and enumerations for commonly accessed engine datatypes.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include "engine_types.h"


namespace enginetype {

    /**
     * Array of all actions.
     */
    const Action ALL_ACTIONS[NUM_MV] = {Action::right, Action::down, Action::left, Action::up, Action::noop};

    /**
     * Array of all actions without NOOP
     * This is useful when running pathfinding at the grid level where we don't want to stand still.
     */
    const Action ALL_ACTIONS_NO_NOOP[NUM_MV-1] = {Action::right, Action::down, Action::left, Action::up};
}