/**
 * @file: controller.cpp
 *
 * @brief: Controller interface which every controller is based on.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "controller.h"

// Engine
#include "../engine/engine_helper.h" 

#include "../util/timer.h" 

// Logging
#include "../util/statistics.h"
#include "../util/logger.h"
#include <plog/Logger.h>

// Controllers
#include "controller_listing.h"
#include "default/default.h"
#include "mcts/mcts.h"
#include "replay/replay.h"
#include "two_level_search/two_level_search.h"


OptionSingleStep noopOption(Action::noop, 1);


/*
 * Default constructor which gets the controller type from the engine
 */
Controller::Controller() {
    initController();
    step_counter_ = 0;
}


/*
 * Constructor which sets the controller based on a given controller type
 * Used for testing
 */
Controller::Controller(ControllerType controller) {
    initController(controller);
    step_counter_ = 0;
}


/*
 * Reset the controller
 * HandleLevelStart is called. 
 */
void Controller::reset() {
    PLOGI_(logger::FileLogger) << "Resetting controller.";
    PLOGI_(logger::ConsoleLogger) << "Resetting controller.";
    PLOGI_(logger::FileLogger) << baseController_.get()->controllerDetailsToString();

    // Set sprite IDs
    enginehelper::initSpriteIDs();

    // Reset available options
    // Needs to be done here (after the level and sprites are loaded.)
    enginehelper::setSimulatorFlag(true);
    baseController_.get()->resetOptions();

    // Solution is cleared and stored with noops to begin
    currentAction_.clear();
    nextAction_ = Action::noop;
    step_counter_ = 0;

    // Controller specific handler to ensure proper setup
    baseController_.get()->handleLevelStart();
    enginehelper::setSimulatorFlag(false);
}


/**
 * Check if the controller wants to request a reset.
 */
bool Controller::requestReset() {
    return baseController_->requestRest();
}


void Controller::handleLevelSolved() {
    // Log info to user
    PLOGI_(logger::FileLogger) << "Game solved.";
    PLOGI_(logger::ConsoleLogger) << "Game solved.";

    // Stats
    statistics::numLevelTries += 1;

    PLOGI_(logger::FileLogger) << "Number of plays = " << statistics::numLevelTries;
    PLOGI_(logger::ConsoleLogger) << "Number of plays = " << statistics::numLevelTries;

    // Signal game over and close logs
    enginehelper::setEngineGameStatusModeQuit();
    logger::closeReplayFile();
}


void Controller::handleLevelFailed() {
    PLOGI_(logger::FileLogger) << "Game Failed.";
    PLOGI_(logger::ConsoleLogger) << "Game Failed.";

    statistics::numLevelTries += 1;

    if (baseController_.get()->retryOnLevelFail()) {
        // Handle necessary changes before/after level reload
        baseController_.get()->handleLevelRestartBefore();
        enginehelper::restartLevel();
        logger::savePlayerMove("reset");
        reset();
        baseController_.get()->handleLevelRestartAfter();
    }
}


/*
 * Get the action from the controller.
 * currentSolution_ is the action which the agent is currently performing. forwardSolution_
 * is the action which the controller is planning to take once the current action is 
 * complete.
 */
Action Controller::getAction() {
    enginehelper::setSimulatorFlag(true);
    Action action = Action::noop;
    BaseController* baseController = baseController_.get();

    // Handle empty action solution queue.
    if (currentAction_.empty()) {
        // If both currentAction_ and nextAction_ are empty, and controller 
        // wants to use forward model to plan while executing, it should seed with noop
        currentAction_.insert(currentAction_.end(), enginetype::ENGINE_RESOLUTION, baseController_.get()->getAction());
    }

    // Continue to run controller to find the next option which should be taken.
    baseController->plan();

    // Get next action in action-currentSolution_ queue
    if (!currentAction_.empty()) {
        action = currentAction_.front();
        currentAction_.erase(currentAction_.begin());
    }

    // Send action information to logger
    // We only care about information at the engine resolution
    if (step_counter_++ % enginetype::ENGINE_RESOLUTION == 0) {
        // Save action to replay file
        logger::savePlayerMove(actionToString(action));

        logger::logPlayerMove(actionToString(action));
        logger::logCurrentState();
        logger::logBoardSpriteIDs();
    }

    enginehelper::setSimulatorFlag(false);

    return action;
}


/*
 * Inits the controller.
 * Controller type is determined by command line argument.
 */
void Controller::initController() {
    enginehelper::initZorbristTables();
    initController(enginehelper::getControllerType());
}


/*
 * Inits the controller.
 * Controller type is determined by command line argument.
 */
void Controller::initController(ControllerType controller) {
    // reset statistics
    statistics::resetAllStatistics();

    // Set appropriate controller
    if (controller == CONTROLLER_DEFAULT) {
        baseController_ = std::make_unique<Default>();
    }
    else if (controller == CONTROLLER_REPLAY) {
        baseController_ = std::make_unique<Replay>();
    }
    else if (controller == CONTROLLER_MCTS) {
        baseController_ = std::make_unique<MCTS>();
    }
    else if (controller == CONTROLLER_MCTS_CUSTOM) {
        baseController_ = std::make_unique<MCTS>(OptionFactoryType::CUSTOM);
    }
    else if (controller == CONTROLLER_TWOLEVEL) {
        baseController_ = std::make_unique<TwoLevelSearch>(OptionFactoryType::TWO_LEVEL_SEARCH);
    }
    // Add case for new ControllerType and initialize baseController_
    else {
        PLOGE_(logger::FileLogger) << "Unknown controller type: " << controller;
        PLOGE_(logger::ConsoleLogger) << "Unknown controller type: " << controller;
        baseController_ = std::make_unique<Default>();
    }
}