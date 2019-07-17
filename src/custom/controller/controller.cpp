

#include "controller.h"

Controller::Controller() {
    setController(enginehelper::getControllerType());
    step_counter = 0;
    clearSolution();
}


Controller::Controller(enginetype::ControllerType controller) {
    setController(controller);
    step_counter = 0;
    clearSolution();
}


unsigned int Controller::getRunTime() {
    return statistics[enginetype::RUN_TIME];
}


unsigned int Controller::getCountExpandedNodes() {
    return statistics[enginetype::COUNT_EXPANDED_NODES];
}


unsigned int Controller::getCountSimulatedNodes() {
    return statistics[enginetype::COUNT_SIMULATED_NODES];
}


unsigned int Controller::getMaxDepth() {
    return statistics[enginetype::MAX_DEPTH];
}


void Controller::clearSolution() {
    // This doesn't seem right, but we need things to be set before initially running
    // MCTS 
    // reset statistics
    step_counter = 0;
    statistics[enginetype::RUN_TIME] = 0;
    statistics[enginetype::COUNT_EXPANDED_NODES] = 0;
    statistics[enginetype::COUNT_SIMULATED_NODES] = 0;
    statistics[enginetype::MAX_DEPTH] = 0;

    currentSolution.clear();
    forwardSolution.clear();
    for (int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        forwardSolution.push_back(Action::noop);
    }
    // mcts.reset(forwardSolution);
}


Action Controller::getAction() {
    Action action = Action::noop;
    std::string msg = "";

    // If current game is over, clean up file handler (if still open)
    // and send the default action of noop.
    if (enginehelper::engineGameFailed() || enginehelper::engineGameSolved()) {
        logwrap::closeSaveRunFile();
        return action;
    }

    // Log current state of game
    msg = "Current state before selecting action:";
    PLOGD_(logwrap::FileLogger) << msg;
    logwrap::logPlayerDetails();
    logwrap::logBoardState();
    logwrap::logMovPosState();
    logwrap::logMovDirState();


    // Handle empty action solution queue
    if (currentSolution.empty()) {
       baseController.get()->handleEmpty(currentSolution, forwardSolution);
    }

    // Continue to get solution.
    // Specific implementations will behave slightly different.
    // MCTS will continue with the current tree, BFS will do nothing if 
    // currentSolution is already populated.
    baseController.get()->run(currentSolution, forwardSolution, statistics);

    // Get next action in action-currentSolution queue
    action = currentSolution.front();
    std::string action_str = actionToString(action);
    currentSolution.erase(currentSolution.begin());

    // Send action information to logger
    msg = "Controller sending action: " + action_str;
    PLOGD_(logwrap::FileLogger) << msg;
    PLOGD_IF_(logwrap::ConsolLogger, step_counter % enginetype::ENGINE_RESOLUTION == 0) << msg;

    // if (controller == enginetype::REPLAY) {
    //     return action;
    // }

    logwrap::savePlayerMove(action_str);

    step_counter += 1;

    return action;
}


void Controller::setController(enginetype::ControllerType controller) {
    std::string msg;
    // Controllers handle 
    if (controller == enginetype::REPLAY) {
        baseController = std::make_unique<Replay>();
    }
    else if (controller == enginetype::BFS) {
        baseController = std::make_unique<BFS>();
    }
    else if (controller == enginetype::MCTS) {
        baseController = std::make_unique<MCTS>();
    }
    else {
        // Throw error
        msg = "Unknown controller type: " + std::to_string(controller);
        PLOGE_(logwrap::FileLogger) << msg;
        PLOGE_(logwrap::ConsolLogger) << msg;
    }
}