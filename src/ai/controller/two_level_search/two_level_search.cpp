/**
 * @file: two_level_search.cpp
 *
 * @brief: Masters thesis controller.
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */


#include "two_level_search.h"

#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <bitset>

// Logger
#include "../../util/logger.h"
#include <plog/Logger.h>

/**
 * Convey any important details about the controller in string format.
 */
std::string TwoLevelSearch::controllerDetailsToString() {
    return "Two level search controller.";
}


void logPath(const std::deque<enginetype::GridCell> &path) {
    std::string msg;
    for (auto const & cell : path) {
        msg += "{" + std::to_string(cell.x) + ", " + std::to_string(cell.y) + "}, ";
    }
    PLOGD_(logger::ConsoleLogger) << msg;
    PLOGD_(logger::FileLogger) << msg;
}


void TwoLevelSearch::resetOptions() {
    if (highlevelPlannedPath_.empty()) {
        availableOptions_ = optionFactory_.createOptions(optionFactoryType_);
    }
}


/**
 * Handle necessary items before the level gets restarted.
 */
void TwoLevelSearch::handleLevelRestartBefore() {
    // If we fail, we need to remember to save the state 
    // for the path we were just attempting
    AbstractGameState state;
    state.setFromEngineState();
    storedPathStates_[hash] = state;
}


/**
 * Increment the binary number 'num'
 * If 'num' exceeds the value of having the numBits LSBs set,
 * 'num' will roll over back to zero.
 */
uint64_t incrementRollingBitNumber(uint64_t num, uint64_t numBits) {
    uint64_t maxNum = (1 << numBits) - 1;
    return (num == maxNum ? 0 : num + 1);
}


/**
 * Check if binary number 'num' has bit set at position 'pos'
 * Assumes left most bit is LSB, and indexing starts at 0
 */
bool isBitSet(uint64_t num, int pos) {
    return (num & (1 << pos)) != 0;
}


/**
 * Set new restrictions on the cells which need to be avoided during low level A*
 * The bitmasks represents the current restricted cells from the total set of restrictions.
 * Here, we simply incremenet the bitmask and set the restrictions. 
 */
void TwoLevelSearch::setNewRestrictions() {
    // Clear previous restrictions
    for (size_t i = 0; i < restrictedCellsByOption_.size(); i++) {
        restrictedCellsByOption_[i].clear();
    }

    // Check if we finished previous restrictions and we need to add the newer ones
    // which were found while executing the previous set
    bool flag = true;
    for (size_t i = 0; i < bitMasks_.size(); i++) {
        if (incrementRollingBitNumber(bitMasks_[i], (uint64_t)spritesMoved[i].size()) != 0) {
            flag = false;
            break;
        }
    }
    if (flag) {
        PLOGD_(logger::ConsoleLogger) << "Previous restrictions exhausted, adding newer ones found";
        PLOGD_(logger::FileLogger) << "Previous restrictions exhausted, adding newer ones found";
        spritesMoved = spritesMovedTemp;
        for (size_t i = 0; i < bitMasks_.size(); i++) {
            bitMasks_[i] = 0;
        }
    }

    // Update mask for next access
    for (size_t i = 0; i < bitMasks_.size(); i++) {
        if (i == 0 || bitMasks_[i - 1] == 0) {
            bitMasks_[i] = incrementRollingBitNumber(bitMasks_[i], (uint64_t)spritesMoved[i].size());
        }
    }

    // Log bit masks for debugging
    PLOGD_(logger::ConsoleLogger) << "Bits";
    for (size_t i = 0; i < bitMasks_.size(); i++) {
        PLOGD_(logger::ConsoleLogger) << std::bitset<10>(bitMasks_[i]);
        PLOGD_(logger::FileLogger) << std::bitset<10>(bitMasks_[i]);
    }
    
    // Add constraints based on bitmasks
    PLOGD_(logger::ConsoleLogger) << "Adding restrictions:";
    PLOGD_(logger::FileLogger) << "Adding restrictions:";
    for (size_t i = 0; i < bitMasks_.size(); i++) {
        int constIndex = 0;
        for (auto const & constraint : spritesMoved[i]) {
            if (isBitSet(bitMasks_[i], constIndex)) {
                PLOGD_(logger::ConsoleLogger) << "Step " << i << ", sprite " << constraint.spriteID 
                    << ", x = " << constraint.cell.x << ", y = " << constraint.cell.y ;
                PLOGD_(logger::FileLogger) << "Step " << i << ", sprite " << constraint.spriteID 
                    << ", x = " << constraint.cell.x << ", y = " << constraint.cell.y ;
                restrictedCellsByOption_[i].push_back(constraint.cell);
            }
            constIndex += 1;
        }
    }
}

/**
 * Handle necessary items after the level gets restarted.
 *  
 */
void TwoLevelSearch::handleLevelRestartAfter() {
    requestReset_ = false;
    solutionIndex_ = -1;

    if (enginehelper::getOptParam() == 1) {
        setNewRestrictions();
        return;
    }

    AbstractGameState current;
    current.setFromEngineState();

    // Continue until we have a set of restrictions which results in paths not seen before
    while (true) {
        current.restoreEngineState();
        setNewRestrictions();

        // Set restrictions, generate paths by A* and check if attempted before
        bool flag = true;
        for (std::size_t i = 0; i < highlevelPlannedPath_.size(); i++) {
            BaseOption *option = highlevelPlannedPath_[i];
            
            // Set restricted cells and run A*
            option->setRestrictedCells(restrictedCellsByOption_[i]);
            option->runAStar(enginehelper::getPlayerPosition(), enginehelper::getSpriteGridCell(option->getSpriteID()));
            std::deque<enginetype::GridCell> optionhighlevelPlannedPath_ = option->getSolutionPath();

            // An empty path implies there was a failure when attempting, so we can auto skip
            if (optionhighlevelPlannedPath_.empty()) {break;}

            // Always add player starting position, (this may no longer be needed as empty case handled above)
            optionhighlevelPlannedPath_.push_front(enginehelper::getPlayerPosition());
            
            // Hash the path and check if we have seen before
            uint64_t innerHash = enginehelper::gridcellPathToHash(optionhighlevelPlannedPath_);
            if (storedPathStates_.find(innerHash) == storedPathStates_.end()) {
                flag = false;
                PLOGE_(logger::ConsoleLogger) << "Path not in map " << innerHash << " " << option->toString();
                PLOGE_(logger::FileLogger) << "Path not in map " << innerHash << " " << option->toString();
                logPath(optionhighlevelPlannedPath_);
                break;
            }

            PLOGD_(logger::ConsoleLogger) << "Path in map " << innerHash << " " << option->toString();
            PLOGD_(logger::FileLogger) << "Path in map " << innerHash << " " << option->toString();
            logPath(optionhighlevelPlannedPath_);
            
            // Set engine state so we can correctly test next option path
            storedPathStates_[innerHash].restoreEngineState();
        }

        // If flag is true, then complete path has been tested before
        current.restoreEngineState();
        if (!flag) {break;}

        PLOGD_(logger::ConsoleLogger) << "Skipping restriction permutation";
        PLOGD_(logger::FileLogger) << "Skipping restriction permutation";
    }
    
}


void TwoLevelSearch::highLevelSearch() {
    HighLevelNode startNode = {nullptr, nullptr, nullptr, 0, (double)enginehelper::getLevelRemainingGemsNeeded()};

    // Search data structures
    std::unordered_map<BaseOption*, HighLevelNode> open;
    std::unordered_map<BaseOption*, HighLevelNode> closed;

    // Initialize with start node
    open[startNode.id] = startNode;

    while (!open.empty()) {
        // Pull next node and update data structures
        HighLevelNode node = {nullptr, nullptr, nullptr, std::numeric_limits<float>::lowest(), 0};
        for (auto it = open.begin(); it != open.end(); ++it) {
            if (it->second.g + it->second.h > node.g + node.h) {
                node = it->second;
            }
        }

        if (node.g == std::numeric_limits<float>::lowest()) {
            PLOGE_(logger::ConsoleLogger) << "Cannot find a node to pull";
            break;
        }

        open.erase(node.id);
        closed[node.id] = node;

        PLOGD_(logger::ConsoleLogger) << "Pulling option: " << (node.option == nullptr ? "Root" : node.option->toString());

        // Goal condition check
        // Goal is if we reach the exit
        if (node.option && node.option->getOptionType() == OptionType::ToExit) {
            PLOGD_(logger::ConsoleLogger) << "Found solution.";
            highlevelPlannedPath_.clear();
            while(node.id) {
                highlevelPlannedPath_.push_front(node.option);
                node = closed[node.parentId];
            }
            return;
        }

        // Expand children (each option)
        for (auto const & option : availableOptions_) {
            double dg = enginehelper::getLevelRemainingGemsNeeded();
            
            // if an option is not valid, then we skip
            // OptionToExit -> Must have enough gems collected in history
            if (option->getOptionType() == OptionType::ToExit) {
                HighLevelNode current = node;
                int gemsNeeded = enginehelper::getLevelRemainingGemsNeeded();
                while (current.id) {
                    if (current.option->getOptionType() == OptionType::ToCollectibleSprite) {
                        enginetype::GridCell cell = enginehelper::getSpriteGridCell(current.option->getSpriteID());
                        gemsNeeded -= enginehelper::getItemGemCount(cell);
                    }
                    current = closed[current.parentId];
                }
                // If not enough diamonds were collected in our path, then we cannot go to exit
                if (gemsNeeded > 0) {continue;}
            }
            // OptionCollectibleSprite -> Must have not previously collected the item
            else if (option->getOptionType() == OptionType::ToCollectibleSprite) {
                HighLevelNode current = node;
                int spriteId = option->getSpriteID();
                dg = enginehelper::getItemGemCount(enginehelper::getSpriteGridCell(spriteId));
                bool hasSeen = false;
                while (current.id) {
                    // We have collected this collectible previously
                    if (current.option->getOptionType() == OptionType::ToCollectibleSprite && current.option->getSpriteID() == spriteId) {
                        hasSeen = true;
                        break;
                    }
                    current = closed[current.parentId];
                }
                if (hasSeen) {continue;}
            }
            // Otherwise we don't know how to handle
            else {
                continue;
            }

            PLOGD_(logger::ConsoleLogger) << "Expanding option: " << option->toString();

            double newG = node.g + dg;
            double h = std::max(enginehelper::getLevelRemainingGemsNeeded() - newG, 0.0);

            // Node generated but not expanded
            if (open.find(option) != open.end()) {
                // Check if new path cheaper
                if (open[option].g > newG) {continue;}
                open.erase(option);
            }
            // Node already expanded
            else if (closed.find(option) != closed.end()) {
                // Check if new path cheaper
                if (closed[option].g > newG) {continue;}
                closed.erase(option);
            }
            open[option] = {option, node.id, option, newG, h};
        }
    }
}


/**
 * Handle setup required at level start.
 * 
 * Called only during level start. Any preprocessing or intiailizations needed for the 
 * controller that wouldn't otherwise be done during each game tick, should be setup here.
 */
void TwoLevelSearch::handleLevelStart() {
    // Current sprites
    PLOGD_(logger::ConsoleLogger) << "Number of options: " << availableOptions_.size();
    PLOGD_(logger::FileLogger) << "Number of options: " << availableOptions_.size();
    for (auto const & option : availableOptions_) {
        PLOGD_(logger::ConsoleLogger) << option->toString() << " " << (option->isValid() ? "true" : "false");
        PLOGD_(logger::FileLogger) << option->toString() << " " << (option->isValid() ? "true" : "false");
    }

    // Reset
    solutionIndex_ = -1;
    lowlevelPlannedPath_.clear();

    // First time attemtping level with this high level path
    if (highlevelPlannedPath_.empty()) {
        PLOGI_(logger::ConsoleLogger) << "Resetting high level path";
        PLOGI_(logger::FileLogger) << "Resetting high level path";

        highLevelSearch();

        // Clear restricted cells for each option in the high level path
        restrictedCellsByOption_.clear();
        restrictedCellsByOption_.insert(restrictedCellsByOption_.end(), highlevelPlannedPath_.size(), {});

        // Clear the sprites moved detection map for 
        spritesMovedTemp.clear();
        spritesMovedTemp.insert(spritesMovedTemp.end(), highlevelPlannedPath_.size(), {});
        spritesMoved = spritesMovedTemp;
        
        // Reset bit masks for which restrictions to consider
        bitMasks_.clear();
        bitMasks_.insert(bitMasks_.end(), highlevelPlannedPath_.size(), 0);

        // Cached paths
        storedPathStates_.clear();
    }

    // Log high level macro option path
    PLOGI_(logger::ConsoleLogger) << "Solution length: " << highlevelPlannedPath_.size();
    PLOGI_(logger::FileLogger) << "Solution length: " << highlevelPlannedPath_.size();
    for (auto const & option : highlevelPlannedPath_) {
        PLOGD_(logger::ConsoleLogger) << option->toString();
        PLOGD_(logger::FileLogger) << option->toString();
    }

    // Log restricted cells
    PLOGD_(logger::ConsoleLogger) << "Cells with restrictions: " << highlevelPlannedPath_.size();
    PLOGD_(logger::FileLogger) << "Cells with restrictions: " << highlevelPlannedPath_.size();
    for (std::size_t i = 0; i < spritesMoved.size(); i++) {
        for (auto const & constraint : spritesMoved[i]) {
            PLOGD_(logger::ConsoleLogger) << "Step " << i << ", sprite " << constraint.spriteID 
            << ", x = " << constraint.cell.x << ", y = " << constraint.cell.y ;
            PLOGD_(logger::FileLogger) << "Step " << i << ", sprite " << constraint.spriteID 
            << ", x = " << constraint.cell.x << ", y = " << constraint.cell.y ;
        }
    }

    requestReset_ = false;
    optionStatusFlag_ = true;
    prevIsMoving_.clear();
    currIsMoving_.clear();
}


/**
 * Check for newely moved objects as a result of player actions
 */
void TwoLevelSearch::checkForMovedObjects() {
    prevPlayerCell_ = currPlayerCell_;
    currPlayerCell_ = enginehelper::getPlayerPosition();
    prevIsMoving_ = currIsMoving_;
    currIsMoving_.clear();

    // Get all sprite grid locations
    std::vector<enginetype::GridCell> spriteLocations = enginehelper::getMapSprites();
    for (auto const & cell : spriteLocations) {
        int spriteID = enginehelper::getSpriteID(cell);
        currIsMoving_[spriteID] = enginehelper::isMoving(cell);

        // Sprite is new on this step
        if (prevIsMoving_.find(spriteID) == prevIsMoving_.end()) {continue;}

        // Sprite is now moving on this step
        SpriteRestriction spriteRestriction = {spriteID, prevPlayerCell_};
        // if (!prevIsMoving_[spriteID] && currIsMoving_[spriteID] && std::find(spritesMovedTemp[solutionIndex_].begin(), 
        //     spritesMovedTemp[solutionIndex_].end(), spriteRestriction) == spritesMovedTemp[solutionIndex_].end()) 

        // {
        //     spritesMovedTemp[solutionIndex_].push_back(spriteRestriction);
        // }
        if (!prevIsMoving_[spriteID] && currIsMoving_[spriteID] && std::find(spritesMoved[solutionIndex_].begin(), 
            spritesMoved[solutionIndex_].end(), spriteRestriction) == spritesMoved[solutionIndex_].end()) 
            
        {
            spritesMoved[solutionIndex_].push_back(spriteRestriction);
        }
    }
    
}


/**
 * Get the action from the controller.
 */
Action TwoLevelSearch::getAction() {
    // Construct a high level plan over the macro-actions (options)
    // A* (add to algorithms)
    // First ordering is by points (diamonds = 1)
    // Next ordering is by length to get there (A* solution length)
    // int gemsNeeded = enginehelper::getLevelRemainingGemsNeeded();

    // Set currentOption to the option we want to use
    PLOGD_(logger::ConsoleLogger) << "Solution index: " << solutionIndex_  << ", solution size: "  << highlevelPlannedPath_.size();
    PLOGD_(logger::FileLogger) << "Solution index: " << solutionIndex_  << ", solution size: "  << highlevelPlannedPath_.size();

    // We don't have any more options...
    if (solutionIndex_ >= (int)highlevelPlannedPath_.size()) {
        PLOGE_(logger::FileLogger) << "No more options in solution list.";
        PLOGE_(logger::ConsoleLogger) << "No more options in solution list.";
        return Action::noop;
    }

    // Check if we need to poll the next option. 
    if (optionStatusFlag_) {
        // Get next option and set restricted cells
        solutionIndex_ += 1;
        currentOption_ = highlevelPlannedPath_[solutionIndex_];
        currentOption_->setRestrictedCells(restrictedCellsByOption_[solutionIndex_]);
        optionStatusFlag_ = false;

        PLOGI_(logger::FileLogger) << "Next option to execute: " << currentOption_->toString();
        PLOGI_(logger::ConsoleLogger) << "Next option to execute: " << currentOption_->toString();

        // Get path that the option wants to initially run using the current state of the world
        AbstractGameState state;
        state.setFromEngineState();

        storedPathStates_[hash] = state;
        if (storedPathStates_.find(hash) == storedPathStates_.end()) {
            PLOGD_(logger::FileLogger) << "Adding new hash " << hash;
            logPath(lowlevelPlannedPath_);
            storedPathStates_[hash] = state;
        }


        currentOption_->runAStar(enginehelper::getPlayerPosition(), enginehelper::getSpriteGridCell(currentOption_->getSpriteID()));
        lowlevelPlannedPath_ = currentOption_->getSolutionPath();
        lowlevelPlannedPath_.push_front(enginehelper::getPlayerPosition());

        // Set hash for the proposed path, set resulting state once this option is complete
        hash = enginehelper::gridcellPathToHash(lowlevelPlannedPath_);
        if (storedPathStates_.find(hash) == storedPathStates_.end()) {
            PLOGD_(logger::ConsoleLogger) << "Adding new hash " << hash;
            PLOGD_(logger::FileLogger) << "Adding new hash " << hash;
            logPath(lowlevelPlannedPath_);
        }

        // If option is not valid, we cannot progress further, and so set request reset flag
        if (!currentOption_->isValid() && !currentOption_->isComplete()) {
            PLOGE_(logger::FileLogger) << "Option invalid, requesting reset";
            PLOGE_(logger::ConsoleLogger) << "Option invalid, requesting reset";
            requestReset_ = true;
        }
    }

    // Get next option
    Action action = Action::noop;
    optionStatusFlag_ = true;
    if (!currentOption_->isComplete()) {
        if (currentOption_->isValid()) {
            optionStatusFlag_ = currentOption_->getNextAction(action);
        }
        else {
            requestReset_ = true;
        }
    }

    return action;
}


/**
 * Continue to find the next option the agent should take.
 * 
 *
 * @param currentOption Option which the agent gets to execute.
 * @param nextOption Planned option for the agent to take at the future state.
 */
void TwoLevelSearch::plan() {
    checkForMovedObjects();
}