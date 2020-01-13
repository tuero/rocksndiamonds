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
#include "logger.h"


void TwoLevelSearch::addNewConstraints() {
    bool newConstraints = false;
    for (auto const & option : availableOptions_) {
        std::vector<enginetype::GridCell> &optionRestrictions = restrictedCellsByOption_[option];
        for (auto const & restriction : spritesMoved[option]) {
            // Add restriction if cell isn't already restricted
            bool cellRestricted = std::find(optionRestrictions.begin(), optionRestrictions.end(), restriction.cell) != optionRestrictions.end();
            if (!cellRestricted) {
                newConstraints = true;
                optionRestrictions.push_back(restriction.cell);
            }
        }
    }

    // No new constraints added
    if (!newConstraints) {return;}

    // Log total changes
    int count = 0;
    for (auto const & option : availableOptions_) {
        count += restrictedCellsByOption_[option].size();
    }
    PLOGE_(logger::FileLogger) << "New constraints added, total = " << count;
    PLOGE_(logger::ConsoleLogger) << "New constraints added, total = " << count;
}


bool playerCausedSpriteMove(const enginetype::GridCell playerCell, const enginetype::GridCell spriteCell) {
    bool isBelow = (playerCell.y - spriteCell.y == 1 && playerCell.x - spriteCell.x == 0);
    bool isBelowLeft = (playerCell.y - spriteCell.y == 1 && playerCell.x - spriteCell.x == -1);
    bool isBelowRight = (playerCell.y - spriteCell.y == 1 && playerCell.x - spriteCell.x == 1);
    return isBelow || isBelowLeft || isBelowRight;
}


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
    prevSprites_ = currSprites_;
    currSprites_.clear();
    for (auto const & cell : spriteLocations) {
        int spriteID = enginehelper::getSpriteID(cell);
        currIsMoving_[spriteID] = enginehelper::isMoving(cell);
        currSprites_[spriteID] = cell;

        // Not moving, so skip
        if (!enginehelper::isMoving(cell)) {continue;}

        // Sprite is new on this step
        if (prevIsMoving_.find(spriteID) == prevIsMoving_.end()) {continue;}
        if (prevSprites_.find(spriteID) == prevSprites_.end()) {
            PLOGE_(logger::FileLogger) << "Sprite not found, this shouldn't happen.";
            PLOGE_(logger::ConsoleLogger) << "Sprite not found, this shouldn't happen.";
            continue;
        }

        // Player not near moving sprite
        if (!playerCausedSpriteMove(prevPlayerCell_, prevSprites_[spriteID])) {continue;}

        // Sprite is now moving on this step
        // Add if we haven't added before
        SpriteRestriction spriteRestriction = {spriteID, prevPlayerCell_};
        bool alreadyRestricted = std::find(spritesMoved[currentOption_].begin(), spritesMoved[currentOption_].end(), spriteRestriction) != spritesMoved[currentOption_].end();
        if (!prevIsMoving_[spriteID] && currIsMoving_[spriteID] && !alreadyRestricted) {
            spritesMoved[currentOption_].push_back(spriteRestriction);
        }
    }
    
}