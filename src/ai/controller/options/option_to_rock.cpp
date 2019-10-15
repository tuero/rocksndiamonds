/**
 * @file: option_to_sprite.cpp
 *
 * @brief: An option which paths to the given sprite location.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#include "option_to_rock.h"

#include <queue>
#include <map>
#include <iostream>

#include "option_types.h"

// Engine
#include "../../engine/engine_helper.h"

//Logging
#include "../../util/logging_wrapper.h"
#include <plog/Log.h>  


OptionToRock::OptionToRock(int spriteID, Action direction) {
    optionType_ = OptionType::ToRock;
    spriteID_ = spriteID;
    enginetype::GridCell cell = enginehelper::getSpriteGridCell(spriteID);
    item_ = enginehelper::getGridItem(cell);
    direction_ = direction;
    timesCalled_ = 0;

    optionStringName_ += enginetype::TYPE_TO_STRING[item_] + " " + std::to_string(spriteID_);
    optionStringName_ += " " + actionToString(direction_);
}


void OptionToRock::findRock() {
    for (enginetype::GridCell cell : enginehelper::getMapSprites()) {
        if (enginehelper::getGridItem(cell) == enginetype::FIELD_BOULDER) {
            goalCell_ = cell;
            if (direction_ == Action::left) {goalCell_.x -= 1;}
            else if (direction_ == Action::right) {goalCell_.x += 1;}
            else if (direction_ == Action::up) {goalCell_.y -= 1;}
            else if (direction_ == Action::down) {goalCell_.y += 1;}
            break;
        }
    }

    enginetype::GridCell playerCell = enginehelper::getPlayerPosition();
    if (goalCell_.y == playerCell.y - 1 && std::abs(goalCell_.x - playerCell.x) == 1) {
        goalCell_.y = playerCell.y;
    }
}

bool OptionToRock::rockMoved() {
    for (enginetype::GridCell cell : enginehelper::getMapSprites()) {
        if (enginehelper::getGridItem(cell) == enginetype::FIELD_BOULDER) {
            return goalCell_ == cell;
        }
    }
    return false;
}


bool OptionToRock::run() {
    int loopCounter = 0;
    while (true) {
        if (enginehelper::engineGameOver()) {return false;}

        goalCell_ = enginehelper::getSpriteGridCell(spriteID_);
        if (direction_ == Action::left) {goalCell_.x -= 1;}
        else if (direction_ == Action::right) {goalCell_.x += 1;}
        else if (direction_ == Action::up) {goalCell_.y -= 1;}
        else if (direction_ == Action::down) {goalCell_.y += 1;}
        
        runAStar(false);
        if (solutionPath.empty()) {break;}

        enginetype::GridCell cell = solutionPath.front();
        enginetype::GridCell playerCell = enginehelper::getPlayerPosition();
        solutionPath.pop_front();
        Action action = enginehelper::getResultingAction(playerCell, cell);
        
        enginehelper::setEnginePlayerAction(action);
        enginehelper::engineSimulate();

        if (solutionPath.empty()) {break;}

        if (loopCounter == 100) {
            PLOGE_(logwrap::ConsolLogger) << "Loop not terminating.";
            PLOGE_(logwrap::FileLogger) << "Loop not terminating.";
        }
        loopCounter += 1;
    }

    timesCalled_ += 1;
    return true;
}


bool OptionToRock::singleStep(Action &action) {
    action = Action::noop;
    goalCell_ = enginehelper::getSpriteGridCell(spriteID_);
    if (direction_ == Action::left) {goalCell_.x -= 1;}
    else if (direction_ == Action::right) {goalCell_.x += 1;}
    else if (direction_ == Action::up) {goalCell_.y -= 1;}
    else if (direction_ == Action::down) {goalCell_.y += 1;}

    enginetype::GridCell playerCell = enginehelper::getPlayerPosition();
    if (playerCell == goalCell_) {
        return true;
    }

    if (enginehelper::getGridMovPos(enginehelper::getSpriteGridCell(spriteID_)) != 0) {
        return false;
    }

    runAStar(false);

    // Get next gridcell
    enginetype::GridCell cell = solutionPath.front();
    solutionPath.pop_front();

    // Find the corresponding action
    action = enginehelper::getResultingAction(playerCell, cell);

    // Option is complete if we pulled the last action off the solution.
    return solutionPath.empty();

}


bool OptionToRock::isValid_() {
    if (!enginehelper::isSpriteActive(spriteID_)) {return false;}
    
    goalCell_ = enginehelper::getSpriteGridCell(spriteID_);
    enginetype::GridCell rockCell = enginehelper::getSpriteGridCell(spriteID_);
    enginetype::GridCell playerCell = enginehelper::getPlayerPosition();

    // Fix location for side of rock we want to be on
    if (direction_ == Action::left) {goalCell_.x -= 1;}
    else if (direction_ == Action::right) {goalCell_.x += 1;}
    else if (direction_ == Action::up) {goalCell_.y -= 1;}
    else if (direction_ == Action::down) {goalCell_.y += 1;}

    // Position on side of rock is out of bounds
    if (goalCell_.x < 0 || goalCell_.x >= enginehelper::getLevelWidth() ||
        goalCell_.y < 0 || goalCell_.y >= enginehelper::getLevelHeight()) 
    {
        return false;
    }
    
    // Rock is currently moving
    if (enginehelper::getGridMovPos(rockCell) != 0) {
        return false;
    }

    // Something is currently in the spot we wish to be in (we can't get there)
    int itemType = enginehelper::getGridItem(goalCell_);
    if (itemType != enginetype::FIELD_EMPTY && itemType != enginetype::FIELD_DIRT) {
        return false;
    }

    // We are already at the location
    return !(playerCell == goalCell_);
}


std::string OptionToRock::optionToString() {
    return optionStringName_;
}


// https://stackoverflow.com/questions/1549930/c-equivalent-of-javas-tostring
std::ostream& OptionToRock::toString(std::ostream& o) const {
    return o << optionStringName_;
}


void OptionToRock::setString(const std::string str) {
    optionStringName_ = str;
}