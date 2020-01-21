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

// Standard Libary/STL
#include <string>

// Includes
#include "logger.h"


OptionCollectibleSprite::OptionCollectibleSprite(int spriteID) {
    optionType_ = OptionType::ToCollectibleSprite;
    spriteID_ = spriteID;
    goalCell_ = enginehelper::getSpriteGridCell(spriteID);
    item_ = enginehelper::getGridElement(goalCell_);
    optionStringName_ = enginehelper::getItemReadableDescription(item_) + " " + std::to_string(spriteID_) + ", score = " 
        + std::to_string(enginehelper::getItemScore(goalCell_)) + ", gem count = " + std::to_string(enginehelper::getItemGemCount(goalCell_));
    solutionPath_.clear();
}


/**
 * Convert to human readable format (spriteID, location, etc.)
 */
std::string OptionCollectibleSprite::toString() const {
    return optionStringName_;
}


/**
 * Run the action(s) defined by the option to get to the collectible sprite.
 */
bool OptionCollectibleSprite::run() {
    int MAX_ATTEMPT = 1000;
    int counter = 0;
    Action action = Action::noop;

    while(true) {
        ++counter;

        // Game over while running option
        if (enginehelper::engineGameOver() || MAX_ATTEMPT) {return false;}

        bool flag = getNextAction(action);
        enginehelper::setEnginePlayerAction(action);
        enginehelper::engineSimulate();
        
        if (flag) {return true;}
    }
}


/**
 * Get the next action to get to the collectible sprite.
 */
bool OptionCollectibleSprite::getNextAction(Action &action) {
    action = Action::noop;

    bool isComplete = enginehelper::getPlayerPosition() == goalCell_ || !enginehelper::isSpriteActive(spriteID_);
    if (isComplete) {
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
    return (solutionPath_.empty() && enginehelper::isPlayerDoneAction());
}


/**
 * Checks if the player can walk to the collectible sprite.
 * Player can only walk to the sprite if it is pathable.
 */
bool OptionCollectibleSprite::isValid() {
    goalCell_ = enginehelper::getSpriteGridCell(spriteID_);
    runAStar();
    return !(solutionPath_.empty() && enginehelper::isPlayerDoneAction());
}
