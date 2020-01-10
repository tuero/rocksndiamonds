/**
 * @file: option_to_exit.cpp
 *
 * @brief: An option which paths to the exit door.
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */


#include "option_to_exit.h"

// Includes
#include "logger.h"


OptionToExit::OptionToExit(int spriteID) {
    optionType_ = OptionType::ToExit;
    spriteID_ = spriteID;
    goalCell_ = enginehelper::getSpriteGridCell(spriteID);
    item_ = enginehelper::getGridElement(goalCell_);
    count_ = 0;
    solutionPath_.clear();
}


std::string OptionToExit::toString() const {
    return enginehelper::getItemReadableDescription(item_) + " " + std::to_string(spriteID_);
}


bool OptionToExit::run() {

}


bool OptionToExit::getNextAction(Action &action) {
    if (enginehelper::isExitOpening(goalCell_)) {
        action = Action::noop;
        return false;
    }

    goalCell_ = enginehelper::getSpriteGridCell(spriteID_);
    runAStar();

    if (solutionPath_.empty()) {
        PLOGE_(logger::FileLogger) << "Not able to find path to collectible sprite.";
        PLOGE_(logger::ConsoleLogger) << "Not able to find path to collectible sprite.";
        action = Action::noop;
        return false;
    }

    // Get next gridcell
    enginetype::GridCell cell = solutionPath_.front();
    solutionPath_.pop_front();

    // Find the corresponding action
    enginetype::GridCell playerCell = enginehelper::getPlayerPosition();
    action = enginehelper::getActionFromNeighbours(playerCell, cell);

    // Option is complete if we pulled the last action off the solution.
    return playerCell == goalCell_ && enginehelper::isPlayerDoneAction();
    // return !(enginehelper::isExitOpen(goalCell_) || enginehelper::isExitOpening(goalCell_)) && !(solutionPath_.empty() && enginehelper::isPlayerDoneAction());
}


/**
 * Checks if the player can walk to the exit.
 * Player can only walk to the exit if the exit is open 
 * and the exit is pathable.
 */
bool OptionToExit::isValid() {
    runAStar();
    bool doorValid = enginehelper::isExitOpen(goalCell_) || enginehelper::isExitOpening(goalCell_) || enginehelper::isExitClosing(goalCell_);
    bool playerInGoal = enginehelper::getPlayerPosition() == goalCell_;
    return doorValid && (!solutionPath_.empty() || playerInGoal);
}
