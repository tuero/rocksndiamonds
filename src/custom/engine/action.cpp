
#include "action.h"


/*
 * Get string representation of action
 */
std::string actionToString(const Action action) {
    std::string action_str = "";
    if (action == Action::left) {action_str = "left";}
    else if (action == Action::right) {action_str = "right";}
    else if (action == Action::up) {action_str = "up";}
    else if (action == Action::down) {action_str = "down";}
    else {action_str = "noop";}
    return action_str;
}


/*
 * Get cardinal direction opposite to the input action
 */
Action actionGetOpposite(const Action action) {
    if (action == Action::left) {return Action::right;}
    else if (action == Action::right) {return Action::left;}
    else if (action == Action::up) {return Action::down;}
    else if (action == Action::down) {return Action::up;}
    else {return Action::noop;}
}