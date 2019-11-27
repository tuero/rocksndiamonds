/**
 * @file: ai_entry.cpp
 *
 * @brief: Exposed functions which are called in game engine
 * 
 * @author: Jake Tuero
 * Date: June 2019
 * Contact: tuero@ualberta.ca
 */

#include "ai_entry.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// engine
#include "engine/action.h"
#include "engine/engine_helper.h"
#include "engine/engine_types.h"

// controller
#include "controller/controller.h"

// Misc
#include "level_programming/level_programming.h"
#include "util/rng.h"

// Logging
#include "util/logger.h"
#include <plog/Log.h>

// Tests
#include "tests/test_engine_speed.h"
#include "tests/test_rng.h"


// External variables accessible to engine
// Used disable screenbuffer writing while not in headless mode
// (speeds up engine simulation)
boolean is_simulating;

// Global objects which are needed outside local scopes
Controller controller;


// ------------------------ Init Functions --------------------------

/*
 * Perform all necessary actions at level start.
 */
extern "C" void handleLevelStart() {
    // If level is a custom programmed level, the respective level start code is called.
    levelprogramming::customLevelProgrammingStart();

    // Initialize zorbrist tables for state hashing
    enginehelper::initZorbristTables();

    // clear solution
    controller.reset();

    // Initial logging
    PLOGI_(logger::FileLogger) << "Level starting: " << enginehelper::getLevelNumber();
    PLOGI_(logger::ConsoleLogger) << "Level starting: " << enginehelper::getLevelNumber();
    PLOGI_(logger::FileLogger) << "Gems needed: " << enginehelper::getLevelGemsNeeded();
    PLOGI_(logger::ConsoleLogger) << "Gems needed: " << enginehelper::getLevelGemsNeeded();
    logger::logCurrentState(plog::debug);

    // Ensure RNG seeds reset during level start
    RNG::resetToEngineSeed();
    RNG::resetToSimulationSeed();

    // Initialize and open replay file
    // Don't create replay file if we are currently in a replay
    if (enginehelper::getControllerType() != CONTROLLER_REPLAY) {
        logger::initReplayFile(enginehelper::getLevelSet(), enginehelper::getLevelNumber());
    }
}


/**
 * Call controller action handler when level is solved. 
 * This will close logging and cleanup.
 */
extern "C" void handleLevelSolved() {
    controller.handleLevelSolved();
}


/**
 * Call controller action handler when level is failed. 
 * Depending on the controller, this will either terminate or attempt the level again.
 */
extern "C" void handleLevelFailed() {
    controller.handleLevelFailed();
}


/*
 * Initalizes the controller to be used.
 * The controller type is provided by a command line argument, which the 
 * engine helper grabs.
 */
extern "C" void initController() {
    controller.initController();
}


/*
 * Initialize the loggers.
 * Passes the command line arguments to loggerper
 */
extern "C" void initLogger(int argc, char *argv[]) {
    std::vector<std::string> allArgs(argv, argv + argc);
    std::string programArgs;

    // Convert args vector to string
    // args used will be logged
    for (std::vector<int>::size_type i = 0; i < allArgs.size(); i++) {
        programArgs += allArgs[i] + " ";
    }

    logger::initLogger(static_cast<logger::LogLevel>(options.log_level), programArgs);
}


/*
 * Set the levelset given by the command line argument.
 */
extern "C" void setLevelSet(void) {
    enginehelper::setLevelSet();
} 


/**
 * Set the level
 */
extern "C" void setLevel(int levelNumber) {
    enginehelper::loadLevel(levelNumber);
}

// ----------------------- Action Handler --------------------------

/**
 * Check if the controller wants to request a reset.
 */
extern "C" int requestReset() {
    return controller.requestReset();
}


/*
 * Get an action from the controller and send back to engine.
 * Implementation of solution will depend on controller type.
 */
extern "C" int getAction() {
    return controller.getAction();
}


/*
 * Some custom levels have elements that continuously spawn in
 * Hook needs to be made in event loop, as these features are not supported
 * in the built in CE programming
 */
extern "C" void handleCustomLevelProgramming() {
    levelprogramming::customLevelProgrammingUpdate();
}


// ----------------------------- RNG ------------------------------

/*
 * Wrapper to get random number for engine use.
 */
extern "C" int getRandomNumber(int max) {
    return RNG::getRandomNumber(max);
}



// ------------------------ Tests ------------------------------

/*
 * Test the engine simulator speed.
 * These tests will simulate random player actions and progress the environment
 * forward. Optimizations are blocked engine code that is not needed during simulation,
 * Such as drawing to screen buffers.
 * Results are logged to file
 */
extern "C" void testEngineSpeed() {
    logger::setLogLevel(logger::LogLevel::debug);
    // testenginespeed::testEngineSpeedNoOptimizations();
    testenginespeed::testEngineSpeedWithOptimizations();
}


/*
 * Tests the speed of running MCTS
 * This does one sweep of MCTS with a budget of 20ms (as per engine spec of
 * having a 20ms frame delay).
 * Results are logged to file
 */
extern "C" void testMCTSSpeed() {
    logger::setLogLevel(logger::LogLevel::debug);
    testenginespeed::testMctsSpeed();
}


/*
 * Tests for RNG reproducibility after engine simulations during rollouts
 */
extern "C" void testRNGAfterSimulations() {
    logger::setLogLevel(logger::LogLevel::debug);
    testrng::testStateAfterSimulations();
}

/*
 * Runs all of the above tests in sequence
 * Results are logged to file.
 */
extern "C" void runTests() {
    // Override logging level for tests
    logger::setLogLevel(logger::LogLevel::debug);

    PLOGI_(logger::FileLogger) << "Running all tests...";

    // Save current game state
    GameState state;
    state.setFromEngineState();

    // enginetype::GridCell exitCell = enginehelper::findExitLocation();
    // enginehelper::setBoardDistancesDijkstra(exitCell);

    // state.restoreEngineState();
    // testenginespeed::testEngineSpeedNoOptimizations();

    state.restoreEngineState();
    testenginespeed::testEngineSpeedWithOptimizations();
    
    // state.restoreEngineState();
    // testenginespeed::testBfsSpeed();

    // state.restoreEngineState();
    // testenginespeed::testMctsSpeed();

    // state.restoreEngineState();
    // testrng::testStateAfterSimulations();
}

