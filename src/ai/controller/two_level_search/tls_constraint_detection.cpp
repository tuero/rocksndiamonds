/**
 * @file: tls_constraint_detection.cpp
 *
 * @brief: Constraint detection for Masters thesis controller.
 * 
 * @author: Jake Tuero
 * Date: January 2020
 * Contact: tuero@ualberta.ca
 */


#include "two_level_search.h"

// Standard Libary/STL
#include <cmath>

// Includes
#include "engine_types.h"
#include "engine_helper.h"



/**
 * Check for newely moved objects as a result of player actions
 */
void TwoLevelSearch::checkForMovedObjects() {
    prevPlayerCell_ = currPlayerCell_;
    currPlayerCell_ = enginehelper::getPlayerPosition();
    prevIsMoving_ = currIsMoving_;
    currIsMoving_.clear();

    // First move, skip.
    if (prevPlayerCell_ == (enginetype::GridCell){-1, -1}) {return;}

    // Get all sprite grid locations
    std::vector<enginetype::GridCell> spriteLocations = enginehelper::getMapSprites();
    for (auto const & cell : spriteLocations) {
        int spriteID = enginehelper::getSpriteID(cell);
        currIsMoving_[spriteID] = enginehelper::isMoving(cell);

        // Not moving, so skip
        if (!enginehelper::isMoving(cell)) {continue;}

        // Sprite is new on this step
        if (prevIsMoving_.find(spriteID) == prevIsMoving_.end()) {continue;}

        // Player not near moving sprite
        bool isBelow = (prevPlayerCell_.y - cell.y == 1 && prevPlayerCell_.x - cell.x == 0);
        bool isBelowLeft = (prevPlayerCell_.y - cell.y == 1 && prevPlayerCell_.x - cell.x == -1);
        bool isBelowRight = (prevPlayerCell_.y - cell.y == 1 && prevPlayerCell_.x - cell.x == 1);
        if (!isBelow && !isBelowLeft && !isBelowRight) {continue;}

        // Sprite is now moving on this step
        // Add if we haven't added before
        SpriteRestriction spriteRestriction = {spriteID, prevPlayerCell_};
        bool alreadyRestricted = std::find(spritesMoved[currentOption_].begin(), spritesMoved[currentOption_].end(), spriteRestriction) != spritesMoved[currentOption_].end();
        if (!prevIsMoving_[spriteID] && currIsMoving_[spriteID] && !alreadyRestricted) {
            spritesMoved[currentOption_].push_back(spriteRestriction);
        }
    }
    
}