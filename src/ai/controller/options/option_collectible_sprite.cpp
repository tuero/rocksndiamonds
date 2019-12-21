/**
 * @file: option_collectible_sprite.cpp
 *
 * @brief: An option which paths to the given sprite location.
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */


#include "option_collectible_sprite.h"

// Includes
#include "logger.h"


OptionCollectibleSprite::OptionCollectibleSprite(int spriteID) {
    optionType_ = OptionType::ToCollectibleSprite;
    spriteID_ = spriteID;
    goalCell_ = enginehelper::getSpriteGridCell(spriteID);
    item_ = enginehelper::getGridElement(goalCell_);
    count_ = 0;
    optionStringName_ = enginehelper::getItemReadableDescription(item_) + " " + std::to_string(spriteID_) + ", score = " 
        + std::to_string(enginehelper::getItemScore(goalCell_)) + ", gem count = " + std::to_string(enginehelper::getItemGemCount(goalCell_));
    solutionPath_.clear();
}


std::string OptionCollectibleSprite::toString() const {
    return optionStringName_;
}


bool OptionCollectibleSprite::run() {

}


bool OptionCollectibleSprite::getNextAction(Action &action) {
    action = Action::noop;

    if (isComplete()) {
        return true;
    }

    // Update goal/player positions
    goalCell_ = enginehelper::getSpriteGridCell(spriteID_);
    enginetype::GridCell playerCell = enginehelper::getPlayerPosition();
    
    runAStar();

    // Get next gridcell
    if (!solutionPath_.empty()) {
        enginetype::GridCell cell = solutionPath_.front();
        solutionPath_.pop_front();

        // Find the corresponding action
        action = enginehelper::getActionFromNeighbours(playerCell, cell);
    }

    // Option is complete if we pulled the last action off the solution.
    // return solutionPath_.empty();
    // return playerCell == goalCell_ && enginehelper::isPlayerDoneAction();
    return (solutionPath_.empty() && enginehelper::isPlayerDoneAction());
}


bool OptionCollectibleSprite::isValid_() {
    runAStar();
    return !(solutionPath_.empty() && enginehelper::isPlayerDoneAction());
}


bool OptionCollectibleSprite::isComplete() {
    return enginehelper::getPlayerPosition() == goalCell_ || !enginehelper::isSpriteActive(spriteID_);
}