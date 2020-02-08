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

// Standard Libary/STL
#include <iostream>

// Includes
#include "engine_helper.h" 
#include "timer.h" 
#include "logger.h"

#include "statistics.h"

// Controllers
#include "controller_listing.h"
#include "option_types.h"
#include "default/default.h"
#include "mcts/mcts.h"
#include "replay/replay.h"
#include "two_level_search/two_level_search.h"


/**
 * Default constructor which gets the controller type from the engine
 */
Controller::Controller() {
    initController();
    step_counter_ = 0;
}


/**
 * Constructor which sets the controller based on a given controller type
 * Used for testing
 */
Controller::Controller(ControllerType controller) {
    initController(controller);
    step_counter_ = 0;
}


/**
 * Called when the level is started for the first time.
 * Special initializations in which we only want to occur once.
 */
void Controller::handleFirstLevelStart() {
    statistics::numLevelTries += 1;

    // Initialize controller options
    baseController_.get()->initializeOptions();

    // Controller specific handler to ensure proper setup
    baseController_.get()->handleLevelStart();

    reset();
}


/**
 * Reset the controller
 * This is called during first level start and every level reattempt.
 */
void Controller::reset() {
    PLOGD_(logger::FileLogger) << "Resetting controller.";
    PLOGD_(logger::ConsoleLogger) << "Resetting controller.";

    actionsTaken_.clear();

    // Reset available options
    baseController_.get()->resetOptions();

    // Solution is cleared and stored with noops to begin
    currentAction_.clear();
    step_counter_ = 0;
}


/**
 * Check if the controller wants to request a reset.
 */
bool Controller::requestReset() {
    return baseController_->requestReset();
}


/**
 * Called when level is solved.
 * This will close logging and cleanup.
 */
void Controller::handleLevelSolved() {
    // Log info to user
    PLOGI_(logger::FileLogger) << "Game solved.";
    PLOGI_(logger::ConsoleLogger) << "Game solved.";

    // Stats
    statistics::numLevelTries += 1;

    PLOGI_(logger::FileLogger) << "Number of plays = " << statistics::numLevelTries;
    PLOGI_(logger::ConsoleLogger) << "Number of plays = " << statistics::numLevelTries;

    // Signal game over and close logs
    logger::createReplayForIndividualRun(enginehelper::getLevelSet(), enginehelper::getLevelNumber(), actionsTaken_);
    enginehelper::setEngineGameStatusModeQuit();
    logger::closeReplayFile();
}


/**
 * Called when level is failed. 
 * Depending on the controller, this will either terminate or attempt the level again.
 */
void Controller::handleLevelFailed() {
    static const int MSG_FREQ = 100;
    statistics::numLevelTries += 1;

    if (baseController_.get()->retryOnLevelFail()) {
        // Handle necessary changes before/after level reload
        baseController_.get()->handleLevelRestartBefore();
        if (statistics::numLevelTries % MSG_FREQ == 0) {
            PLOGI_(logger::FileLogger) << "Restarting level: " << enginehelper::getLevelNumber() << ", attempt " << statistics::numLevelTries;
            PLOGI_(logger::ConsoleLogger) << "Restarting level: " << enginehelper::getLevelNumber() << ", attempt " << statistics::numLevelTries;
        }
        enginehelper::restartLevel();
        logger::savePlayerMove("reset");
        reset();
        baseController_.get()->handleLevelRestartAfter();
    }
}


/**
 * Get the action from the controller.
 * currentSolution_ is the action which the agent is currently performing. forwardSolution_
 * is the action which the controller is planning to take once the current action is 
 * complete.
 */
int Controller::getAction() {
    enginehelper::setSimulatorFlag(true);
    Action action = Action::noop;

    try{
        BaseController* baseController = baseController_.get();

        // Handle empty action solution queue (ask controller for next action)
        if (currentAction_.empty()) {
            currentAction_.insert(currentAction_.end(), enginetype::ENGINE_RESOLUTION, baseController_.get()->getAction());
        }

        // Continue to run controller to find the next option which should be taken.
        // Controller implementation may be to do nothing (if we are not using forward model planning).
        baseController->plan();

        // Get next action in queue
        if (!currentAction_.empty()) {
            action = currentAction_.front();
            currentAction_.erase(currentAction_.begin());
        }

        // Send action information to logger
        // We only care about information at the engine resolution
        if (step_counter_++ % enginetype::ENGINE_RESOLUTION == 0) {
            // Save action to replay file
            logger::savePlayerMove(enginehelper::actionToString(action));
            logger::logPlayerMove(enginehelper::actionToString(action));
            logger::logCurrentState();
            actionsTaken_.push_back(enginehelper::actionToString(action));
        }
    } 
    catch (const std::exception &ex) {
        std::cerr << "A failure occured: " << ex.what() << ". Please check logs." << std::endl;
    }
    catch (...) {
        // catch any errors
        std::cerr << "Unknown failure occurred. Please check logs." << std::endl;
    }

    enginehelper::setSimulatorFlag(false);

    // Send action to engine, which expects as the underlying int code.
    return static_cast<std::underlying_type_t<Action>>(action);
}


/**
 * Inits the controller.
 * Controller type is determined by command line argument.
 */
void Controller::initController() {
    enginehelper::initZorbristTables();
    initController(enginehelper::getControllerType());
}


/**
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
    else if (controller == CONTROLLER_MCTS_OPTIONS) {
        baseController_ = std::make_unique<MCTS>(OptionFactoryType::PATH_TO_SPRITE);
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
    PLOGI_(logger::FileLogger) << "Using controller: " << baseController_.get()->controllerDetailsToString();
    PLOGI_(logger::ConsoleLogger) << "Using controller: " << baseController_.get()->controllerDetailsToString();
}