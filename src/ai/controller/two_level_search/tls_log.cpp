/**
 * @file: tls_log.cpp
 *
 * @brief: Logging functions for Masters thesis controller.
 * 
 * @author: Jake Tuero
 * Date: January 2020
 * Contact: tuero@ualberta.ca
 */


#include "two_level_search.h"


// Includes
#include "logger.h"


void TwoLevelSearch::logPath(const std::deque<enginetype::GridCell> &path) {
    std::string msg;
    for (auto const & cell : path) {
        msg += "{" + std::to_string(cell.x) + ", " + std::to_string(cell.y) + "}, ";
    }
    PLOGD_(logger::ConsoleLogger) << msg;
    PLOGD_(logger::FileLogger) << msg;
}


void TwoLevelSearch::logAvailableOptions() {
    PLOGD_(logger::ConsoleLogger) << "Number of options: " << availableOptions_.size();
    PLOGD_(logger::FileLogger) << "Number of options: " << availableOptions_.size();
    for (auto const & option : availableOptions_) {
        PLOGD_(logger::ConsoleLogger) << option->toString() << " " << (option->isValid() ? "true" : "false");
        PLOGD_(logger::FileLogger) << option->toString() << " " << (option->isValid() ? "true" : "false");
    }
}


void TwoLevelSearch::logHighLevelPath() {
    PLOGI_(logger::ConsoleLogger) << "Solution length: " << highlevelPlannedPath_.size();
    PLOGI_(logger::FileLogger) << "Solution length: " << highlevelPlannedPath_.size();
    for (auto const & option : highlevelPlannedPath_) {
        PLOGD_(logger::ConsoleLogger) << option->toString();
        PLOGD_(logger::FileLogger) << option->toString();
    }
}

void TwoLevelSearch::logRestrictedSprites() {
    PLOGD_(logger::ConsoleLogger) << "Cells with restrictions: " << highlevelPlannedPath_.size();
    PLOGD_(logger::FileLogger) << "Cells with restrictions: " << highlevelPlannedPath_.size();
    for (auto const & option : availableOptions_) {
        PLOGD_(logger::ConsoleLogger) << "Restrictions for option: " << option->toString();
        PLOGD_(logger::FileLogger) << "Restrictions for option: " << option->toString();
        for (auto const & constraint : spritesMoved[option]) {
            PLOGD_(logger::ConsoleLogger) << "sprite " << constraint.spriteID 
            << ", x = " << constraint.cell.x << ", y = " << constraint.cell.y ;
            PLOGD_(logger::FileLogger) << "sprite " << constraint.spriteID 
            << ", x = " << constraint.cell.x << ", y = " << constraint.cell.y ;
        }
    }
}