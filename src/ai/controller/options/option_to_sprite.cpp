/**
 * @file: option_to_sprite.cpp
 *
 * @brief: An option which paths to the given sprite location.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#include "option_to_sprite.h"

#include <queue>
#include <map>
#include <iostream>

#include "option_types.h"

// Engine
#include "../../engine/engine_helper.h"

//Logging
#include "../../util/logging_wrapper.h"
#include <plog/Log.h>  


OptionToSprite::OptionToSprite(int spriteID) {
    optionType_ = OptionType::ToSprite;
    spriteID_ = spriteID;
    enginetype::GridCell cell = enginehelper::getSpriteGridCell(spriteID);
    item_ = enginehelper::getGridItem(cell);
    count_ = 0;
    optionStringName_ += enginetype::TYPE_TO_STRING[item_] + " " + std::to_string(spriteID_);
}


bool OptionToSprite::run() {
    int loopCounter = 0;
    while (true) {
        goalCell_ = enginehelper::getSpriteGridCell(spriteID_);
        runAStar(false);
        if (solutionPath.empty()) {break;}
        enginetype::GridCell cell = solutionPath.front();
        solutionPath.pop_front();
        Action action = enginehelper::getResultingAction(enginehelper::getPlayerPosition(), cell);
        
        enginehelper::setEnginePlayerAction(action);
        enginehelper::engineSimulate();

        if (enginehelper::engineGameOver()) {return false;}
        if (solutionPath.empty()) {break;}

        if (loopCounter == 100) {
            PLOGE_(logwrap::FileLogger) << "Loop not terminating.";
            PLOGE_(logwrap::ConsolLogger) << "Loop not terminating.";
        }
        loopCounter += 1;
    }

    count_ += 1;
    timesCalled_ += 1;
    return true;
}


bool OptionToSprite::singleStep(Action &action) {
    goalCell_ = enginehelper::getSpriteGridCell(spriteID_);
    runAStar(false);

    // Get next gridcell
    enginetype::GridCell cell = solutionPath.front();
    solutionPath.pop_front();

    // Find the corresponding action
    enginetype::GridCell playerCell = enginehelper::getPlayerPosition();
    action = enginehelper::getResultingAction(playerCell, cell);

    // Option is complete if we pulled the last action off the solution.
    return solutionPath.empty();
}


bool OptionToSprite::isValid_() {
    if (!enginehelper::isSpriteActive(spriteID_)) {return false;}

    enginetype::GridCell playerCell = enginehelper::getPlayerPosition();
    enginetype::GridCell spriteCell = enginehelper::getSpriteGridCell(spriteID_);

    return !(playerCell == spriteCell);
}


std::string OptionToSprite::optionToString() {
    return optionStringName_;
}


// https://stackoverflow.com/questions/1549930/c-equivalent-of-javas-tostring
std::ostream& OptionToSprite::toString(std::ostream& o) const {
    return o << optionStringName_;
}


void OptionToSprite::setString(const std::string str) {
    optionStringName_ = str;
}