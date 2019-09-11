/**
 * @file: controller.h
 *
 * @brief: Controller interface which 
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
#include "bfs/bfs.h"
#include "mcts/mcts.h"
#include "replay/replay.h"
#include "user/user.h"
#include "pfa/pfa.h"


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

    // Solution is cleared and stored with noops to begin
    currentSolution_.clear();
    forwardSolution_.clear();
    for (int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        forwardSolution_.push_back(Action::noop);
    }

    // Controller specific handler to ensure proper setup
    baseController_.get()->handleLevelStart();
}


/*
 * Get the action from the controller.
 * currentSolution_ is the action which the agent is currently performing. forwardSolution_
 * is the action which the controller is planning to take once the current action is 
 * complete.
 */
Action Controller::getAction() {
    Action action = Action::noop;
    std::string msg = "";

    // If current game is over, clean up file handler (if still open)
    // and send the default action of noop.
    if (enginehelper::engineGameFailed() || enginehelper::engineGameSolved()) {
        logwrap::closeReplayFile();
        return action;
    }


    // Handle empty action solution queue.
    // Generally, the controller will move forwardSolution_ into currentSolution_.
    if (currentSolution_.empty()) {
       baseController_.get()->handleEmpty(currentSolution_, forwardSolution_);
    }

    // Continue to get solution.
    // Specific implementations will behave slightly different.
    // MCTS will continue with the current tree, BFS will do nothing if 
    // currentSolution_ is already populated.
    baseController_.get()->run(currentSolution_, forwardSolution_, statistics_);
    enginehelper::setSimulatorFlag(false);

    // Get next action in action-currentSolution_ queue
    if (!currentSolution_.empty()) {
        action = currentSolution_.front();
        currentSolution_.erase(currentSolution_.begin());
    }

    std::string actionStr = actionToString(action);

    // Send action information to logger
    // We only care about information at the engine resolution
    if (step_counter_ % enginetype::ENGINE_RESOLUTION == 0) {
        msg = "Controller sending action: " + actionStr;
        PLOGI_(logwrap::FileLogger) << msg;
        PLOGD_(logwrap::ConsolLogger) << msg;

        // Log current state of game
        logwrap::logState();
    }

    // Save action to replay file
    logwrap::savePlayerMove(actionStr);

    step_counter_ += 1;

    return action;
}


/*
 * Set the controller.
 */
void Controller::setController(enginetype::ControllerType controller) {
    if (controller == enginetype::REPLAY) {
        baseController_ = std::make_unique<Replay>();
    }
    else if (controller == enginetype::BFS) {
        baseController_ = std::make_unique<BFS>();
    }
    else if (controller == enginetype::MCTS) {
        baseController_ = std::make_unique<MCTS>();
    }
    else if (controller == enginetype::USER) {
        baseController_ = std::make_unique<User>();
    }
    else if (controller == enginetype::PFA) {
        baseController_ = std::make_unique<PFA>();
    }
    else {
        // Throw error
        std::string msg = "Unknown controller type: " + std::to_string(controller);
        PLOGE_(logwrap::FileLogger) << msg;
        PLOGE_(logwrap::ConsolLogger) << msg;
    }
}