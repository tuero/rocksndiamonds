/**
 * @file: engine_action_info.h
 *
 * @brief: Interface for controllers to access action information from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

// ------------- Includes  -------------

// Standard Libary/STL
#include <string>

// Include
#include "engine_types.h"

namespace enginehelper {
namespace actioninfo {
    
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

} //namespace actioninfo
} //namespace enginehelper