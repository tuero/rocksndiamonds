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

// Standard Libary/STL
#include <queue>
#include <map>
#include <iostream>

// Includes
#include "option_types.h"
#include "engine_helper.h"
#include "logger.h"


OptionToSprite::OptionToSprite(int spriteID) {
    optionType_ = OptionType::ToSprite;
    spriteID_ = spriteID;
    enginetype::GridCell cell = enginehelper::getSpriteGridCell(spriteID);
    item_ = enginehelper::getGridElement(cell);
    count_ = 0;
    optionStringName_ += enginehelper::getItemReadableDescription(item_) + " " + std::to_string(spriteID_);
    solutionPath_.clear();
}


bool OptionToSprite::run() {
    int loopCounter = 0;
    while (true) {
        goalCell_ = enginehelper::getSpriteGridCell(spriteID_);
        runAStar();
        if (solutionPath_.empty()) {break;}
        enginetype::GridCell cell = solutionPath_.front();
        solutionPath_.pop_front();
        Action action = enginehelper::getActionFromNeighbours(enginehelper::getPlayerPosition(), cell);
        
        enginehelper::setEnginePlayerAction(action);
        enginehelper::engineSimulate();

        if (enginehelper::engineGameOver()) {return false;}
        if (solutionPath_.empty()) {break;}

        if (loopCounter == 100) {
            PLOGE_(logger::FileLogger) << "Loop not terminating.";
            PLOGE_(logger::FileLogger) << "Loop not terminating.";
        }
        loopCounter += 1;
    }

    count_ += 1;
    timesCalled_ += 1;
    return true;
}


bool OptionToSprite::getNextAction(Action &action) {
    // Run A* to get path to sprite
    goalCell_ = enginehelper::getSpriteGridCell(spriteID_);
    runAStar();

    // Get next gridcell
    enginetype::GridCell cell = solutionPath_.front();
    solutionPath_.pop_front();

    // Find the corresponding action
    enginetype::GridCell playerCell = enginehelper::getPlayerPosition();
    action = enginehelper::getActionFromNeighbours(playerCell, cell);

    // Option is complete if we pulled the last action off the solution.
    return solutionPath_.empty();
}


bool OptionToSprite::isValid_() {
    // Option to walk to sprite is invalid if sprite not active on map.
    if (!enginehelper::isSpriteActive(spriteID_)) {return false;}

    enginetype::GridCell playerCell = enginehelper::getPlayerPosition();
    enginetype::GridCell spriteCell = enginehelper::getSpriteGridCell(spriteID_);

    return !(playerCell == spriteCell);
}


std::string OptionToSprite::toString() const {
    return optionStringName_;
}


void OptionToSprite::setString(const std::string str) {
    optionStringName_ = str;
}