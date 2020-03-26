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

// Standard Libary/STL
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>

// Pytorch
#include <torch/torch.h>
#include <torch/script.h>

// Includes
#include "util/tls_feature.h"
#include "util/tls_hash.h"
#include "util/tls_feature.h"
#include "statistics.h"
#include "logger.h"

using namespace enginehelper;


/**
 * Convey any important details about the controller in string format.
 */
std::string TwoLevelSearch::controllerDetailsToString() {
    return "Two level search controller.";
}


void TwoLevelSearch::initializeOptions() {
    availableOptions_ = optionFactory_.createOptions(optionFactoryType_);
    for (auto & option : availableOptions_) {
        option->setAvoidNonGoalCollectibleCells(true);
    }
}


/**
 * Reset the options (only if we don't currently have a high level plan)
 */
void TwoLevelSearch::resetOptions() {
    if (highlevelPlannedPath_.empty()) {
        availableOptions_ = optionFactory_.createOptions(optionFactoryType_);
        for (auto & option : availableOptions_) {
            option->setAvoidNonGoalCollectibleCells(true);
        }
    }
}


/**
 * Handle necessary items before the level gets restarted.
 */
void TwoLevelSearch::handleLevelRestartBefore() {
    
}


void TwoLevelSearch::incrementPathTimesVisited() {
    // Statistics logging
    ++(statistics::pathCounts[levelinfo::getLevelNumber()][currentHighLevelPathHash_]);
    statistics::solutionPathCounts[levelinfo::getLevelNumber()][0] = currentHighLevelPathHash_;
    statistics::solutionPathCounts[levelinfo::getLevelNumber()][1] = ++hashPathTimesVisited[currentHighLevelPathHash_];
}


/**
 * Initializations which need to occur BOTH on first level start
 * and on every level restart after a failure.
 */
void TwoLevelSearch::initializationForEveryLevelStart() {
    requestReset_ = false;
    solutionIndex_ = -1;
    optionStatusFlag_ = true;

    // Player tracking for constraint detection
    playerCells_[0] = {-1, -1};
    playerCells_[1] = {-1, -1};

    PLOGD_(logger::FileLogger) << "------------------------";
    highLevelSearch();
    incrementPathTimesVisited();
    // logHighLevelPath();
    // logRestrictedSprites();
}


/**
 * Handle necessary items after the level gets restarted.
 */
void TwoLevelSearch::handleLevelRestartAfter() {
    initializationForEveryLevelStart();
}


/**
 * Handle setup required at first level start.
 * 
 * Called only during level start. Any preprocessing or intiailizations needed for the 
 * controller that wouldn't otherwise be done during each game tick, should be setup here.
 */
void TwoLevelSearch::handleLevelStart() {
    logAvailableOptions();

    initialState.setFromEngineState();

    // Clear and intialize data structures
    hashPathTimesVisited.clear();
    restrictedCellsByOption_.clear();
    restrictedCellsByPath_.clear();
    restrictedCellsByOptionCount_.clear();
    for (auto const & hash : tlshash::allItemsPairHashes(availableOptions_)) {
        restrictedCellsByOption_[hash] = {};
        restrictedCellsByOptionCount_[hash] = 0;
    }


// Manally adding constraints
#ifdef MANUAL_CONSTRAINTS
    // Find rocks (potential restrictions)
    std::vector<enginetype::GridCell> rockCells;
    PLOGE_(logger::FileLogger) << "Rocks";
    for (auto const & cell : gridinfo::getMapSprites()) {
        if (gridinfo::getGridElement(cell) == enginetype::ELEMENT_BD_ROCK) {
            rockCells.push_back({cell.x, cell.y + 1});
            PLOGE_(logger::FileLogger) <<  cell.x << "," << cell.y+1;
        }
    }
    auto addRestrictionsIfApplicable = [&](uint64_t hash, const std::deque<enginetype::GridCell> &solutionPath) {
        for (auto const & cell :solutionPath) {
            if (std::find(rockCells.begin(), rockCells.end(), cell) != rockCells.end()) {
                restrictedCellsByOption_[hash].insert(gridinfo::cellToIndex(cell));
                ++restrictedCellsByOptionCount_[hash];
                PLOGE_(logger::FileLogger) <<  cell.x << "," << cell.y+1;
            }
        }
    };

    // Agent starting position to each highlevel action
    enginetype::GridCell startCell = gridinfo::getPlayerPosition();
    for (auto const & option : availableOptions_) {
        uint64_t hash = tlshash::hashPath(availableOptions_, {option});
        option->runAStar(startCell, option->getGoalCell());
        // Walk along base path and if a rock is above, set as restriction
        // addRestrictionsIfApplicable(hash, option->getSolutionPath());
        for (auto const & cell :option->getSolutionPath()) {
            if (std::find(rockCells.begin(), rockCells.end(), cell) != rockCells.end()) {
                restrictedCellsByOption_[hash].insert(gridinfo::cellToIndex(cell));
                ++restrictedCellsByOptionCount_[hash];
                PLOGE_(logger::FileLogger) << option->toString() << ", (" << cell.x << ", " << cell.y << ")";
            }
        }
    }
    PLOGE_(logger::FileLogger) << "----------------";
    // Pair options
    for (auto & prev : availableOptions_) {
        for (auto & curr : availableOptions_) {
            // Same option means starting position to option, handled above
            if (prev == curr) {continue;}
            uint64_t hash = tlshash::hashPath(availableOptions_, {prev, curr});
            
            // Set path
            curr->runAStar(prev->getGoalCell(), curr->getGoalCell());

            // Walk along base path and if a rock is above, set as restriction
            // addRestrictionsIfApplicable(hash, curr->getSolutionPath());
            for (auto const & cell :curr->getSolutionPath()) {
                if (std::find(rockCells.begin(), rockCells.end(), cell) != rockCells.end()) {
                    restrictedCellsByOption_[hash].insert(gridinfo::cellToIndex(cell));
                    ++restrictedCellsByOptionCount_[hash];
                    PLOGE_(logger::FileLogger) << prev->toString() << " -> " << curr->toString() << ", (" << cell.x << ", " << cell.y << ")";
                }
            }
        }
    }
#endif

    openLevinNodes_.clear(); 
    closedLevinNodes_.clear();

#ifndef SINGLE_PATH

    // Create starting levin node for each 1 step path 
    for (auto const & option : availableOptions_) {
        int numGem = elementproperty::getItemGemCount(gridinfo::getSpriteGridCell(option->getSpriteID()));
        bool hasDoor = elementproperty::isExit(gridinfo::getSpriteGridCell(option->getSpriteID()));
        uint64_t singleStepHash = tlshash::hashPath(availableOptions_, {option});
        int restriction_count = restrictionCountForPath({option});
        openLevinNodes_.insert({{option}, tlsfeature::getNodePath({option}), singleStepHash, 0, 0, 
            restriction_count, CombinatorialPartition(restriction_count), numGem, hasDoor, false, false
        });
    }
#else
    PLOGE_(logger::ConsoleLogger) << availableOptions_.size();
    std::vector<BaseOption*> path{availableOptions_[7], availableOptions_[4], availableOptions_[3], availableOptions_[1], availableOptions_[8],
        availableOptions_[6], availableOptions_[8], availableOptions_[2], availableOptions_[0], availableOptions_[5]
    };
    uint64_t pathHash = tlshash::hashPath(availableOptions_, path);
    setPathRestrictionSet(pathHash, path);
    int restriction_count = restrictedCellsByPath_[pathHash].size();
    openLevinNodes_.insert({path, tlsfeature::getNodePath({option}), pathHash, 0, 0, 
        restriction_count, CombinatorialPartition(restriction_count), 6, true, false
    });
#endif
    initializationForEveryLevelStart();
}


const std::string DATA_DIR = "./src/ai/training_data/";

/**
 * Handle necessary items after the level is solved
 * Here we will dump the training data in tensor format to a zip for python use later
 */
void TwoLevelSearch::handleLevelSolved() {
#ifdef TRAINING
    std::vector<torch::Tensor> featureTensors;
    std::vector<torch::Tensor> observationTensors;

    const int NUM_OPEN = 100;
    const int NUM_CLOSED = 100;


    // Add solution node
    for (auto const node : openLevinNodes_) {
        // Never visited during search, skip
        if (node.timesVisited == 0) {continue;}
        if (node.hash == currentHighLevelPathHash_) {
            featureTensors.push_back(tlsfeature::getNodeFeature(node, initialState.Feld_));
            observationTensors.push_back(tlsfeature::getNodeObservation(node, true, false));
            break;
        }
    }

    // Open nodes
    std::vector<NodeLevin> openVec(openLevinNodes_.begin(), openLevinNodes_.end());
    std::sort(openVec.begin(), openVec.end(), [](const NodeLevin & l, const NodeLevin & r) {return l.timesVisited > r.timesVisited;});
    for (int i = 0; i < std::min(NUM_OPEN, (int)openVec.size()); ++i) {
        featureTensors.push_back(tlsfeature::getNodeFeature(openVec[i], initialState.Feld_));
        observationTensors.push_back(tlsfeature::getNodeObservation(openVec[i], false, false));
    }


    // Closed nodes (exhausted i.e. timeout but not lower bound)
    std::vector<NodeLevin> closedVec(closedLevinNodes_.begin(), closedLevinNodes_.end());
    std::sort(closedVec.begin(), closedVec.end(), [](const NodeLevin & l, const NodeLevin & r) {return l.timesVisited > r.timesVisited;});
    for (int i = 0; i < std::min(NUM_CLOSED, (int)closedVec.size()); ++i) {
        featureTensors.push_back(tlsfeature::getNodeFeature(closedVec[i], initialState.Feld_));
        observationTensors.push_back(tlsfeature::getNodeObservation(closedVec[i], false, true));
    }

    // Save featureTensors for python training
    std::string baseDataFileName = DATA_DIR + levelinfo::getLevelSet() + "_" + std::to_string(levelinfo::getLevelNumber()) + "_";
    std::ofstream ofFeature(baseDataFileName + "feature.zip", std::ios::out | std::ios::binary);
    auto totalFeatureTensor = torch::stack(featureTensors, 0);
    auto bytesFeature = torch::jit::pickle_save(totalFeatureTensor);
    ofFeature.write(bytesFeature.data(), bytesFeature.size());
    ofFeature.close();

    // Save observationTensors for python training
    std::ofstream ofObservation(baseDataFileName + "observation.zip", std::ios::out | std::ios::binary);
    auto totalObservationTensor = torch::stack(observationTensors, 0);
    auto bytesObservation = torch::jit::pickle_save(totalObservationTensor);
    ofObservation.write(bytesObservation.data(), bytesObservation.size());
    ofObservation.close();
#endif
}


/**
 * Get the action from the controller.
 */
Action TwoLevelSearch::getAction() {
    // Check if we need to poll the next option. 
    if (optionStatusFlag_) {
        // Get next option and set restricted cells
        solutionIndex_ += 1;

        // We don't have any more options...
        if (solutionIndex_ >= (int)highlevelPlannedPath_.size()) {
            requestReset_ = true;
            return Action::noop;
        }

        // Get next option
        previousOption_ = (solutionIndex_ == 0) ? highlevelPlannedPath_[solutionIndex_] : currentOption_;
        currentOption_ = highlevelPlannedPath_[solutionIndex_];
        optionStatusFlag_ = false;

        PLOGD_(logger::FileLogger) << "Next option to execute: " << currentOption_->toString();
        PLOGD_(logger::FileLogger) << "Option restrictions: ";
        for (auto const & restriction : currentOption_->getRestrictedCells()) {
            PLOGD_(logger::FileLogger) << "x=" << restriction.x << ", y=" << restriction.y;
        }

        // If option is not valid, we cannot progress further, and so set request reset flag
        if (!currentOption_->isValid()) {
            PLOGD_(logger::FileLogger) << "Option invalid, requesting reset";
            requestReset_ = true;
        }
    }

    // Get next option
    Action action = Action::noop;
    optionStatusFlag_ = true;
    requestReset_ = true;
    if (currentOption_->isValid()) {
        optionStatusFlag_ = currentOption_->getNextAction(action);
        requestReset_ = false;
    }

    return action;
}


/**
 * Use this time to check for moved objects.
 */
void TwoLevelSearch::plan() {
#ifndef MANUAL_CONSTRAINTS
    checkForMovedObjects();
#endif
}