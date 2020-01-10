/**
 * @file: tls_hash.cpp
 *
 * @brief: Path hashing functions for Masters thesis controller.
 * 
 * @author: Jake Tuero
 * Date: January 2020
 * Contact: tuero@ualberta.ca
 */


#include "two_level_search.h"

// Standard Libary/STL
#include <algorithm>
#include <iterator>


// Include
#include "logger.h"


uint64_t TwoLevelSearch::optionPathToHash(std::deque<BaseOption*> path) {
    uint64_t hash = 0;
    uint64_t pow = 10;
    for (auto const & option : path) {
        // Get option index
        auto iter = std::find(availableOptions_.begin(), availableOptions_.end(), option);
        uint64_t index = std::distance(availableOptions_.begin(), iter);
        while (index >= pow) {pow *= 10;}
        hash = (hash * pow) + index;
    }

    return hash;
}


void TwoLevelSearch::findUnseenPath() {
    AbstractGameState current;
    current.setFromEngineState();

    //Continue until we have a set of restrictions which results in paths not seen before
    while (true) {
        current.restoreEngineState();
        // setNewRestrictions();

        // Set restrictions, generate paths by A* and check if attempted before
        bool flag = true;
        for (std::size_t i = 0; i < highlevelPlannedPath_.size(); i++) {
            BaseOption *option = highlevelPlannedPath_[i];
            
            // Set restricted cells and run A*
            // option->setRestrictedCells(restrictedCellsByOption_[i]);
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


void TwoLevelSearch::saveCurrentPathHash() {
    // hash is initial path taken from option which just finished
    // We save the state after path complete for the initial path
    if (hash != 0 && storedPathStates_.find(hash) == storedPathStates_.end()) {
        AbstractGameState state;
        state.setFromEngineState();
        PLOGD_(logger::FileLogger) << "Adding new hash " << hash;
        logPath(lowlevelPlannedPath_);
        storedPathStates_[hash] = state;
    }

    // Get initial path that the new option wants to run
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
}