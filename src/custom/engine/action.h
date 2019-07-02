#ifndef ACTION_H
#define ACTION_H


// ------------- Includes  -------------
#include <array>
#include <string>

#include "engine_types.h"


enum Action {noop=_ENGINE_NOOP, left=_ENGINE_LEFT, right=_ENGINE_RIGHT, up=_ENGINE_UP, down=_ENGINE_DOWN};

// static const std::array<Action, NUM_ACTIONS> ALL_ACTIONS = {Action::right, Action::down, Action::left, Action::up, Action::noop};
static const Action ALL_ACTIONS[] = {Action::right, Action::down, Action::left, Action::up, Action::noop};

/*
 * Get string representation of action
 */
std::string actionToString(const Action action);

/*
 * Get cardinal direction opposite to the input action
 */
Action actionGetOpposite(const Action action);

#endif  //ACTION