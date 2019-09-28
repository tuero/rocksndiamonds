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


/*
 * Get string representation of action.
 */
const std::string actionToString(const Action action) {
    std::string action_str = "noop";
    if (action == Action::left) {action_str = "left";}
    else if (action == Action::right) {action_str = "right";}
    else if (action == Action::up) {action_str = "up";}
    else if (action == Action::down) {action_str = "down";}

    return action_str;
}


/*
 * Get the enum representation of the action.
 */
Action stringToAction(const std::string &str) {
    Action action = Action::noop;
    if (str == "left") {action = Action::left;}
    else if (str == "right") {action = Action::right;}
    else if (str == "up") {action = Action::up;}
    else if (str == "down") {action = Action::down;}

    return action;
}


/*
 * Get action for the cardinal opposite of the given action.
 */
Action actionGetOpposite(const Action action) {
    if (action == Action::left) {return Action::right;}
    else if (action == Action::right) {return Action::left;}
    else if (action == Action::up) {return Action::down;}
    else if (action == Action::down) {return Action::up;}
    else {return Action::noop;}
}