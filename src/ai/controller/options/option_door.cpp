/**
 * @file: option_door.cpp
 *
 * @brief: An option which paths to the coloured door.
 * 
 * @author: Jake Tuero
 * Date: February 2020
 * Contact: tuero@ualberta.ca
 */


#include "option_door.h"

// Standard library and STL
#include <string>

// Includes
#include "engine_types.h"
#include "engine_helper.h"
#include "logger.h"

using namespace enginehelper;


OptionDoor::OptionDoor(int spriteID) {
    optionType_ = OptionType::ToDoor;
    spriteID_ = spriteID;
    goalCell_ = gridinfo::getSpriteGridCell(spriteID);
    item_ = gridinfo::getGridElement(goalCell_);
    solutionPath_.clear();
}


/**
 * Convert to human readable format (spriteID, location, etc.)
 */
std::string OptionDoor::toString() const {
    return elementproperty::getItemReadableDescription(item_) + " " + std::to_string(spriteID_);
}


/**
 * Run the action(s) defined by the option to get to the exit.
 */
bool OptionDoor::run() {
    int MAX_ATTEMPT = 1000;
    int counter = 0;
    Action action = Action::noop;

    while(true) {
        ++counter;

        // Game over while running option
        if (enginestate::engineGameOver() || MAX_ATTEMPT) {return false;}

        bool flag = getNextAction(action);
        enginestate::setEnginePlayerAction(action);
        enginestate::engineSimulate();
        
        if (flag) {return true;}
    }
}


/**
 * Get the next action to get to the exit.
 */
bool OptionDoor::getNextAction(Action &action) {
    if (elementproperty::isGateClosed(goalCell_)) {
        action = Action::noop;
        return false;
    }

    goalCell_ = gridinfo::getSpriteGridCell(spriteID_);
    runAStar();

    if (solutionPath_.empty()) {
        PLOGE_(logger::FileLogger) << "Not able to find path to " << toString();
        PLOGE_(logger::ConsoleLogger) << "Not able to find path to " << toString();
        action = Action::noop;
        return false;
    }

    // Get next gridcell
    enginetype::GridCell cell = solutionPath_.front();
    solutionPath_.pop_front();

    // Find the corresponding action
    enginetype::GridCell playerCell = gridinfo::getPlayerPosition();
    action = gridaction::getActionFromNeighbours(playerCell, cell);

    // Option is complete if we pulled the last action off the solution.
    return solutionPath_.empty();
}


/**
 * Checks if the player can walk to the exit.
 * Player can only walk to the exit if the exit is open and the exit is pathable.
 */
bool OptionDoor::isValid() {
    runAStar();
    bool doorValid = elementproperty::isGateOpen(goalCell_);
    return doorValid && !solutionPath_.empty();
}
