/**
 * @file: action.h
 *
 * @brief: Action enum following engine specification, with related methods.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "action.h"


/**
 * Get string representation of action.
 */
const std::string actionToString(const Action action) {
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
