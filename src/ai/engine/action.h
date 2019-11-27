/**
 * @file: action.h
 *
 * @brief: Action enum following engine specification, with related methods.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef ACTION_H
#define ACTION_H


#include <vector>
#include <string>
#include <unordered_map>

#include "engine_types.h"


/**
 * Action definition, based on internal engine definitions.
 */
enum Action {noop=enginetype::ENGINE_NOOP, left=enginetype::ENGINE_LEFT, right=enginetype::ENGINE_RIGHT, 
    up=enginetype::ENGINE_UP, down=enginetype::ENGINE_DOWN
};

/**
 * Array of all actions.
 */
static const Action ALL_ACTIONS[] = {Action::right, Action::down, Action::left, Action::up, Action::noop};

/**
 * Array of all actions without NOOP
 * This is useful when running pathfinding at the grid level where we don't want to stand still.
 */
static const Action ALL_ACTIONS_NO_NOOP[] = {Action::right, Action::down, Action::left, Action::up};

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
 *
 * @param str The action string
 */
const std::string actionToString(const Action action);

/**
 * Get the enum representation of the action.
 *
 * @param str The action string
 */
Action stringToAction(const std::string &str);

/**
 * Get the opposite direction.
 * 
 * @param action The reference action
 * @return The opposite action direction.
 */
Action getOppositeDirection(Action action);

#endif  //ACTION