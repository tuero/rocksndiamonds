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
#include <array>
#include <string>

#include "engine_types.h"


enum Action {noop=enginetype::ENGINE_NOOP, left=enginetype::ENGINE_LEFT, right=enginetype::ENGINE_RIGHT, 
    up=enginetype::ENGINE_UP, down=enginetype::ENGINE_DOWN
};

// static const std::array<Action, NUM_ACTIONS> ALL_ACTIONS = {Action::right, Action::down, Action::left, Action::up, Action::noop};
static const Action ALL_ACTIONS[] = {Action::right, Action::down, Action::left, Action::up, Action::noop};

static const std::vector<Action> singleStepNoop(enginetype::ENGINE_RESOLUTION, Action::noop);

/*
 * Get string representation of action.
 *
 * @param str The action string
 */
const std::string actionToString(const Action action);

/*
 * Get the enum representation of the action.
 *
 * @param str The action string
 */
Action stringToAction(const std::string &str);

/*
 * Get cardinal direction opposite to the input action.
 *
 * @param action The given action
 */
Action actionGetOpposite(const Action action);

#endif  //ACTION