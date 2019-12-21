/**
 * @file: engine_action_info.cpp
 *
 * @brief: Implementation of action information from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include "engine_helper.h"

// Standard Libary/STL
#include <string>
#include <unordered_map>

// Includes
#include "engine_types.h"

namespace enginehelper {

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
    auto it = ACTION_TO_STRING.find(action);
    return (it == ACTION_TO_STRING.end() ? Action::noop : static_cast<Action>(MV_DIR_OPPOSITE(action)));
}

} //namespace enginehelper