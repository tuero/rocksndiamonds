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

// System libraries
#include <sys/stat.h>           // mkdir
#include <sys/types.h>

// Standard Libary/STL
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <stdlib.h>
#include <time.h>

// Includes
#include "util/tls_feature.h"
#include "util/tls_hash.h"
#include "util/tls_feature.h"
#include "statistics.h"
#include "logger.h"

// Pytorch
#include <torch/torch.h>
#include <torch/script.h>

using namespace enginehelper;


const std::string MODEL_DIR = "./src/ai/distnet/src/export/";
const std::string MODEL_EXT = ".pt";


TwoLevelSearch::TwoLevelSearch(OptionFactoryType optionType, const std::string & modelPath, PolicyType policyType) : BaseController(optionType) {
    try {
        // Device is CPU by default, CUDA if available
        torch::Device device = torch::kCPU;
        if (torch::cuda::is_available()) {
            PLOGE_(logger::FileLogger) << "CUDA is available, using GPU";
            PLOGE_(logger::ConsoleLogger) << "CUDA is available, using GPU";
            device = torch::kCUDA;
        }

        // Deserialize the ScriptModule from a file using torch::jit::load().
        PLOGE_(logger::FileLogger) << "Loading model  " << modelPath;
        PLOGE_(logger::ConsoleLogger) << "Loading model  " << modelPath;
        std::cout << "Tring to load model: " << MODEL_DIR << modelPath << MODEL_EXT << std::endl;
        model_ = torch::jit::load(MODEL_DIR + modelPath + MODEL_EXT, device);
    }
    catch (const c10::Error& e) {
        PLOGE_(logger::FileLogger) << "A failure occured: can't load module.";
        PLOGE_(logger::ConsoleLogger) << "A failure occured: can't load module.";
        std::cerr << "A failure occured: can't load module." << std::endl;
        enginestate::setEngineGameStatusModeQuit();
        exit(1);
    }

    PLOGE_(logger::FileLogger) << "Model " << modelPath << " loaded successfully.";
    PLOGE_(logger::ConsoleLogger) << "Model " << modelPath << " loaded successfully.";

    statistics::output_msg = modelPath;

    // Set policy type
    policyType_ = policyType;
}


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


/**
 * Increment the path counter for stats logging.
 */
void TwoLevelSearch::incrementPathTimesVisited() {
    // Statistics logging
    ++(statistics::pathCounts[levelinfo::getLevelNumber()][currentHighLevelPathHash_]);
    statistics::solutionPathCounts[levelinfo::getLevelNumber()][0] = currentHighLevelPathHash_;
    statistics::solutionPathCounts[levelinfo::getLevelNumber()][1] = ++hashPathTimesVisited_[currentHighLevelPathHash_];
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
    hashPathTimesVisited_.clear();
    restrictedCellsByOption_.clear();
    restrictedCellsByPath_.clear();
    restrictedCellsByOptionCount_.clear();
    for (auto const & hash : tlshash::allItemsPairHashes(availableOptions_)) {
        restrictedCellsByOption_[hash] = {};
        restrictedCellsByOptionCount_[hash] = 0;
    }

    if (policyType_ == PolicyType::Trivial) {
        statistics::output_msg = "Trivial";
    }


// Manally adding constraints
#ifdef MANUAL_CONSTRAINTS
    addManualConstraints();
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

        std::vector<BaseOption*> temp_vec{option};
        openLevinNodes_.insert({{option}, tlsfeature::getNodePath({option}), singleStepHash, 0,
            restriction_count, numGem, hasDoor, false, false,
            policyType_, model_, initialState
        });
    }
#else
    // Single path for testing
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
    std::vector<torch::Tensor> featureTensors_closed;
    std::vector<torch::Tensor> observationTensors_closed;

    const int NUM_OPEN = 50;
    const int NUM_CLOSED = 50;

    // Create data dir if it doesn't exist
    mkdir(DATA_DIR.c_str(), 0755);

    // Walk through nodes, remove nodes which were not visited, and save solution node
    for (auto iter = openLevinNodes_.begin(); iter != openLevinNodes_.end(); ) {
        if (iter->timesVisited == 0) {
            iter = openLevinNodes_.erase(iter);
        }
        else if (iter->hash == currentHighLevelPathHash_) {
            featureTensors.push_back(tlsfeature::getNodeFeature(*iter, initialState.Feld_));
            observationTensors.push_back(tlsfeature::getNodeObservation(*iter, true, false));
            iter = openLevinNodes_.erase(iter);
        }
        else {
            ++iter;
        }
    }

    srand(time(0));

    // Open nodes
    std::vector<NodeLevin> openVec(openLevinNodes_.begin(), openLevinNodes_.end());
    // std::sort(openVec.begin(), openVec.end(), [](const NodeLevin & l, const NodeLevin & r) {return l.timesVisited > r.timesVisited;});
    for (int i = 0; i < std::min(NUM_OPEN, (int)openVec.size()); ++i) {
        int randomIndex = rand() % openVec.size();
        featureTensors.push_back(tlsfeature::getNodeFeature(openVec[randomIndex], initialState.Feld_));
        observationTensors.push_back(tlsfeature::getNodeObservation(openVec[randomIndex], false, false));
        openVec.erase(openVec.begin() + randomIndex);
        // featureTensors.push_back(tlsfeature::getNodeFeature(openVec[i], initialState.Feld_));
        // observationTensors.push_back(tlsfeature::getNodeObservation(openVec[i], false, false));
    }


    // Closed nodes (exhausted i.e. timeout but not lower bound)
    std::vector<NodeLevin> closedVec(closedLevinNodes_.begin(), closedLevinNodes_.end());
    // std::sort(closedVec.begin(), closedVec.end(), [](const NodeLevin & l, const NodeLevin & r) {return l.timesVisited > r.timesVisited;});
    for (int i = 0; i < std::min(NUM_CLOSED, (int)closedVec.size()); ++i) {
        int randomIndex = rand() % closedVec.size();
        featureTensors_closed.push_back(tlsfeature::getNodeFeature(closedVec[randomIndex], initialState.Feld_));
        observationTensors_closed.push_back(tlsfeature::getNodeObservation(closedVec[randomIndex], false, true));
        closedVec.erase(closedVec.begin() + randomIndex);
        // featureTensors_closed.push_back(tlsfeature::getNodeFeature(closedVec[i], initialState.Feld_));
        // observationTensors_closed.push_back(tlsfeature::getNodeObservation(closedVec[i], false, true));
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

    // Save featureTensors of closed list for python training
    std::ofstream ofFeature_closed(baseDataFileName + "feature_closed.zip", std::ios::out | std::ios::binary);
    auto totalFeatureTensor_closed = torch::stack(featureTensors_closed, 0);
    auto bytesFeature_closed = torch::jit::pickle_save(totalFeatureTensor_closed);
    ofFeature_closed.write(bytesFeature_closed.data(), bytesFeature_closed.size());
    ofFeature_closed.close();

    // Save observationTensors of closed list for python training
    std::ofstream ofObservation_closed(baseDataFileName + "observation_closed.zip", std::ios::out | std::ios::binary);
    auto totalObservationTensor_closed = torch::stack(observationTensors_closed, 0);
    auto bytesObservation_closed = torch::jit::pickle_save(totalObservationTensor_closed);
    ofObservation_closed.write(bytesObservation_closed.data(), bytesObservation_closed.size());
    ofObservation_closed.close();
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