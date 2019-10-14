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

// Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   

// Controllers
// #include "bfs/bfs.h"
#include "mcts/mcts.h"
#include "replay/replay.h"
// #include "user/user.h"
// #include "pfa/pfa.h"


OptionSingleStep noopOption(Action::noop, 1);


/*
 * Default constructor which gets the controller type from the engine
 */
Controller::Controller() {
    setController(enginehelper::getControllerType());
    step_counter_ = 0;
}


/*
 * Constructor which sets the controller based on a given controller type
 * Used for testing
 */
Controller::Controller(enginetype::ControllerType controller) {
    setController(controller);
    step_counter_ = 0;
}


/*
 * Get the runtime of the controller.
 */
int Controller::getRunTime() {
    return statistics_[enginetype::RUN_TIME];
}


/*
 * Get the number of nodes expanded by the type of tree search used.
 */
int Controller::getCountExpandedNodes() {
    return statistics_[enginetype::COUNT_EXPANDED_NODES];
}


/*
 * Get the number of nodes simulated by the type of tree search used.
 */
int Controller::getCountSimulatedNodes() {
    return statistics_[enginetype::COUNT_SIMULATED_NODES];
}


/*
 * Get the max depth of nodes expanded by the type of tree search used.
 */
int Controller::getMaxDepth() {
    return statistics_[enginetype::MAX_DEPTH];
}


/*
 * Reset the controller
 * Stored solution is set to NOOP, statistics_ reset, and base controller
 * HandleLevelStart is called. 
 */
void Controller::reset() {
    // reset statistics_
    step_counter_ = 0;
    statistics_[enginetype::RUN_TIME] = 0;
    statistics_[enginetype::COUNT_EXPANDED_NODES] = 0;
    statistics_[enginetype::COUNT_SIMULATED_NODES] = 0;
    statistics_[enginetype::MAX_DEPTH] = 0;

    // Set sprite IDs
    enginehelper::initSpriteIDs();

    // !<-- This needs to be fixed? (multiple factories?)
    baseController_.get()->setAvailableOptions(optionFactory_.createSingleStepOptions());
    if (enginehelper::getControllerType() == enginetype::MCTS_CUSTOM) {
        baseController_.get()->setAvailableOptions(optionFactory_.createCustomOptions());
    }

    // Solution is cleared and stored with noops to begin
    currentSolution_.clear();
    currentOption_ = &noopOption;
    nextOption_ = &noopOption;
    optionStatusFlag_ = true;

    // Controller specific handler to ensure proper setup
    baseController_.get()->handleLevelStart();
}


void Controller::logNextOptionDetails() {
    BaseController* baseController = baseController_.get();
    PLOGI_(logwrap::FileLogger) << "Current option: " << nextOption_->optionToString();
    PLOGD_(logwrap::ConsolLogger) << "Current option: " << nextOption_->optionToString();
    PLOGD_(logwrap::FileLogger) << "Controller details:\n" + baseController->controllerDetailsToString();
    PLOGD_(logwrap::ConsolLogger) << "Controller details:\n" + baseController->controllerDetailsToString();

}

/*
 * Get the action from the controller.
 * currentSolution_ is the action which the agent is currently performing. forwardSolution_
 * is the action which the controller is planning to take once the current action is 
 * complete.
 */
Action Controller::getAction() {
    Action action = Action::noop;
    BaseController* baseController = baseController_.get();
    std::string msg = "";

    // If current game is over, clean up file handler and send NOOP action
    if (enginehelper::engineGameOver()) {
        logwrap::closeReplayFile();
        return action;
    }

    // Handle empty action solution queue.
    if (currentSolution_.empty()) {
        // If the current option is complete, controller should set current option to 
        // next option and handle controller specific resets (forward the search tree).
        if (optionStatusFlag_) {
            logNextOptionDetails();
            baseController->handleEmpty(&currentOption_, &nextOption_);
            optionStatusFlag_ = false;
        }
        // Get next action to take from the current option
        Action tempAction;
        optionStatusFlag_ = currentOption_->singleStep(tempAction);
        currentSolution_.assign(enginetype::ENGINE_RESOLUTION, tempAction);
    }

    // Continue to run controller to find the next option which should be taken.
    baseController->run(&currentOption_, &nextOption_, statistics_);
    enginehelper::setSimulatorFlag(false);

    // Get next action in action-currentSolution_ queue
    if (!currentSolution_.empty()) {
        action = currentSolution_.front();
        currentSolution_.erase(currentSolution_.begin());
    }

    // Send action information to logger
    // We only care about information at the engine resolution
    if (step_counter_++ % enginetype::ENGINE_RESOLUTION == 0) {
        logwrap::logPlayerMove(actionToString(action));
        logwrap::logState();
    }

    // Save action to replay file
    logwrap::savePlayerMove(actionToString(action));

    return action;
}


/*
 * Set the controller.
 */
void Controller::setController(enginetype::ControllerType controller) {
    if (controller == enginetype::REPLAY) {
        baseController_ = std::make_unique<Replay>();
    }
    // else if (controller == enginetype::BFS) {
    //     baseController_ = std::make_unique<BFS>();
    // }
    else if (controller == enginetype::MCTS || controller == enginetype::MCTS_CUSTOM) {
        baseController_ = std::make_unique<MCTS>();
    }
    // else if (controller == enginetype::USER) {
    //     baseController_ = std::make_unique<User>();
    // }
    // else if (controller == enginetype::PFA) {
    //     baseController_ = std::make_unique<PFA>();
    // }
    else if (controller != enginetype::DEFAULT) {
        PLOGE_(logwrap::FileLogger) << "Unknown controller type: " + std::to_string(controller);
        PLOGE_(logwrap::ConsolLogger) << "Unknown controller type: " + std::to_string(controller);
    }
}