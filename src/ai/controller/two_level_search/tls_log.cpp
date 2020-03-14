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
#include "util/tls_hash.h"
#include "logger.h"

using namespace enginehelper;


/**
 * Log the given path of gridcells.
 */
void TwoLevelSearch::logPath(const std::vector<enginetype::GridCell> &path) {
    std::string msg;
    for (auto const & cell : path) {
        msg += "{" + std::to_string(cell.x) + ", " + std::to_string(cell.y) + "}, ";
    }
    PLOGD_(logger::ConsoleLogger) << msg;
    PLOGD_(logger::FileLogger) << msg;
}


/**
 * Log all the available options the controller has access to.
 */
void TwoLevelSearch::logAvailableOptions() {
    PLOGD_(logger::ConsoleLogger) << "Number of options: " << availableOptions_.size();
    PLOGD_(logger::FileLogger) << "Number of options: " << availableOptions_.size();
    for (auto const & option : availableOptions_) {
        PLOGD_(logger::ConsoleLogger) << option->toString() << " " << (option->isValid() ? "true" : "false");
        PLOGD_(logger::FileLogger) << option->toString() << " " << (option->isValid() ? "true" : "false");
    }
}


/**
 * Log the set high level path of options.
 */
void TwoLevelSearch::logHighLevelPath() {
    PLOGD_(logger::FileLogger) << "Solution: " << currentHighLevelPathHash_ << ", length: " << highlevelPlannedPath_.size();
    for (auto const & option : highlevelPlannedPath_) {
        PLOGD_(logger::FileLogger) << option->toString();
    }
}


/**
 * Log all the restricted cells found thus far for each pair of options.
 */
void TwoLevelSearch::logRestrictedSprites() {
    PLOGD_(logger::FileLogger) << "Cells with restrictions: " << highlevelPlannedPath_.size();

    for (auto const & prev : availableOptions_) {
        for (auto const & curr : availableOptions_) {
            uint64_t hash;
            if (prev == curr) {
                hash = tlshash::hashPath(availableOptions_, {prev});
                PLOGD_(logger::FileLogger) << "Restrictions for root to option " << prev->toString();
            }
            else {
                hash = tlshash::hashPath(availableOptions_, {prev, curr});
                PLOGD_(logger::FileLogger) << "Restrictions for option " << prev->toString() << " to option " << curr->toString();
            }
            for (auto const & restriction : restrictedCellsByOption_[hash]) {
                PLOGD_(logger::FileLogger) << "x = " << gridinfo::indexToCell(restriction).x << ", y = " << gridinfo::indexToCell(restriction).y ;
            }
        }
    }
}


void TwoLevelSearch::logLevinNodes() {
    PLOGD_(logger::FileLogger) << "Logging Levin nodes.";
    PLOGD_(logger::FileLogger) << openLevinNodes_.size();
    for (auto const & node : openLevinNodes_) {
        // PLOGD_(logger::FileLogger) << "Node: " << node.hash << ", constraints: " << node.numConstraints 
        //     << " , visited: " << node.timesVisited << ", cost: " << node.cost();
        PLOGD_(logger::FileLogger) << "Node: " << node.hash << ", constraints: " << node.numConstraints 
            << " , visited: " << node.timesVisited;
        for (auto const & option : node.path) {
            PLOGD_(logger::FileLogger) << option->toString();
        }
    }
}