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

using namespace enginehelper;


OptionCollectibleSprite::OptionCollectibleSprite(int spriteID) {
    optionType_ = OptionType::ToCollectibleSprite;
    spriteID_ = spriteID;
    goalCell_ = gridinfo::getSpriteGridCell(spriteID);
    item_ = gridinfo::getGridElement(goalCell_);
    optionStringName_ = elementproperty::getItemReadableDescription(item_) + " " + std::to_string(spriteID_) + ", score = " 
        + std::to_string(elementproperty::getItemScore(goalCell_)) + ", gem count = " + std::to_string(elementproperty::getItemGemCount(goalCell_));
    solutionPath_.clear();
}


/**
 * Convert to human readable format (spriteID, location, etc.)
 */
std::string OptionCollectibleSprite::toString() const {
    return optionStringName_ + " " + std::to_string(spriteID_) + 
        " (x=" + std::to_string(goalCell_.x) + ", y=" + std::to_string(goalCell_.y) + ")";
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
        if (enginestate::engineGameOver() || MAX_ATTEMPT) {return false;}

        bool flag = getNextAction(action);
        enginestate::setEnginePlayerAction(action);
        enginestate::engineSimulate();
        
        if (flag) {return true;}
    }
}


/**
 * Get the next action to get to the collectible sprite.
 */
bool OptionCollectibleSprite::getNextAction(Action &action) {
    action = Action::noop;

    // If we are already at goal cell or sprite has disappeared.
    if (gridinfo::getPlayerPosition() == goalCell_ || !gridinfo::isSpriteActive(spriteID_)) {
        return true;
    }

    // Update goal/player positions
    goalCell_ = gridinfo::getSpriteGridCell(spriteID_);
    enginetype::GridCell playerCell = gridinfo::getPlayerPosition();
    
    runAStar();

    // Get next gridcell
    if (!solutionPath_.empty()) {
        enginetype::GridCell cell = solutionPath_.front();
        solutionPath_.pop_front();

        // Find the corresponding action
        action = gridaction::getActionFromNeighbours(playerCell, cell);
    }

    // Option is complete if we pulled the last action off the solution.
    return (solutionPath_.empty() && enginestate::isPlayerDoneAction());
}


/**
 * Checks if the player can walk to the collectible sprite.
 * Player can only walk to the sprite if it is pathable.
 */
bool OptionCollectibleSprite::isValid() {
    goalCell_ = gridinfo::getSpriteGridCell(spriteID_);
    runAStar();
    return !(solutionPath_.empty() && enginestate::isPlayerDoneAction());
}
