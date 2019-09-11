/**
 * @file: custom.cpp
 *
 * @brief: Exposed functions which are called in game engine
 * 
 * @author: Jake Tuero
 * Date: June 2019
 * Contact: tuero@ualberta.ca
 */

#include "custom.h"

#include <iostream>
#include <string>
#include <vector>

// engine
#include "engine/action.h"
#include "engine/engine_helper.h"
#include "engine/engine_types.h"

// controller
#include "controller/controller.h"

// util
#include "util/level_programming.h"
#include "util/rng.h"
#include "util/summary_window.h"

// Logging
#include "util/logging_wrapper.h"
#include <plog/Log.h>

// Tests
#include "tests/test_engine_speed.h"
#include "tests/test_rng.h"


// External variables accessible to engine
boolean is_simulating;

// Global objects which are needed outside local scopes
std::vector<Action> solution;
Controller controller;

// Level info
char *level_bd = (char *)"classic_boulderdash";
char *level_em = (char *)"classic_emerald_mine";
char *level_custom = (char *)"tuero";
std::string levelsetSurvival = "custom_survival";


std::string SEP(30, '-');



// ------------------------ Init Functions --------------------------

/*
 * Perform all necessary actions at level start.
 */
extern "C" void handleLevelStart() {
    // Log level
    PLOGI_(logwrap::FileLogger) << "Level being played: " << level.file_info.nr;

    // Calculate tile distances to exit location
    // If level is a custom programmed level, the respective level start code is called.
    std::string currentLevelsetSubdir(leveldir_current->subdir);
    if (currentLevelsetSubdir == levelsetSurvival) {
        levelprogramming::customLevelProgrammingStart();
    }
    else {
        enginetype::GridCell exitCell = enginehelper::findExitLocation();
        enginehelper::setBoardDistancesDijkstra(exitCell);
    }

    // Initialize zorbrist tables for state hashing
    enginehelper::initZorbristTables();

    // Send starting board positions and player into to loggers
    logwrap::logLevelStart();

    // clear solution
    controller.reset();

    // Ensure RNG seeds reset during level start
    RNG::setEngineSeed(RNG::getEngineSeed());
    RNG::setSimulatingSeed(RNG::getSimulationSeed());

    if (options.summary_window) {summarywindow::init();}

    // Initialize and open replay file
    logwrap::initReplayFile();
}


/*
 * Set the controller to be used.
 * The controller type is provided by a command line argument, which the 
 * engine helper grabs.
 */
extern "C" void setController() {
    controller.setController(enginehelper::getControllerType());
}


/*
 * Initialize the loggers.
 * Passes the command line arguments to logwrapper
 */
extern "C" void initLogger(int argc, char *argv[]) {
    std::vector<std::string> allArgs(argv, argv + argc);
    std::string programArgs;

    // Convert args vector to string
    // args used will be logged
    for (std::vector<int>::size_type i = 0; i < allArgs.size(); i++) {
        programArgs += allArgs[i] + " ";
    }

    // Set log level depending if debug flag set or if we are in a replay
    plog::Severity logLevel = static_cast<plog::Severity>(options.log_level);
    if (options.controller_type == CONTROLLER_TYPE_REPLAY) {logLevel = plog::error;}

    logwrap::initLogger(logLevel, programArgs);
}


/*
 * Set the levelset given by the command line argument.
 */
extern "C" void setLevelSet(void) {
    enginehelper::setLevelSet();
} 


/*
 * Save the RNG seed, levelset and level used.
 */
extern "C" void saveReplayLevelInfo(void) {
    if (options.controller_type == CONTROLLER_TYPE_REPLAY) {return;}
    logwrap::saveReplayLevelInfo();
}


// ----------------------- Summary Window -------------------------

/*
 * Close the summary window.
 */
extern "C" void closeMapWindow() {
    summarywindow::close();
}

// ----------------------- Action Handler --------------------------

/*
 * Get an action from the controller and send back to engine.
 * Implementation of solution will depend on controller type.
 */
extern "C" int getAction() {
    if (options.summary_window) {summarywindow::draw();}
    return controller.getAction();
}


/*
 * Some custom levels have elements that continuously spawn in
 * Hook needs to be made in event loop, as these features are not supported
 * in the built in CE programming
 */
extern "C" void handleCustomLevelProgramming() {
    std::string currentLevelsetSubdir(leveldir_current->subdir);
    if (currentLevelsetSubdir == levelsetSurvival) {
        levelprogramming::customLevelProgrammingUpdate();
    }
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
    logwrap::setLogLevel(plog::debug);
    testenginespeed::testEngineSpeedNoOptimizations();
    testenginespeed::testEngineSpeedWithOptimizations();
}


/*
 * Tests the speed of running BFS
 * Dijkstra must be ran first to get the tile distances to goal.
 * Results are logged to file
 */
extern "C" void testBFSSpeed() {
    enginetype::GridCell exitCell = enginehelper::findExitLocation();
    enginehelper::setBoardDistancesDijkstra(exitCell);
    logwrap::setLogLevel(plog::debug);
    testenginespeed::testBfsSpeed();
}


/*
 * Tests the speed of running MCTS
 * This does one sweep of MCTS with a budget of 20ms (as per engine spec of
 * having a 20ms frame delay).
 * Results are logged to file
 */
extern "C" void testMCTSSpeed() {
    enginetype::GridCell exitCell = enginehelper::findExitLocation();
    enginehelper::setBoardDistancesDijkstra(exitCell);
    logwrap::setLogLevel(plog::debug);
    testenginespeed::testMctsSpeed();
}


/*
 * Tests for RNG reproducibility after engine simulations during rollouts
 */
extern "C" void testRNGAfterSimulations() {
    logwrap::setLogLevel(plog::debug);
    testrng::testStateAfterSimulations();
}

/*
 * Runs all of the above tests in sequence
 * Results are logged to file.
 */
extern "C" void testAll() {
    // Override logging level for tests
    logwrap::setLogLevel(plog::debug);

    std::string msg = "Running all tests...";
    // Save current game state
    GameState state;
    state.setFromEngineState();

    enginetype::GridCell exitCell = enginehelper::findExitLocation();
    enginehelper::setBoardDistancesDijkstra(exitCell);

    PLOGI_(logwrap::FileLogger) << msg;

    state.restoreEngineState();
    testenginespeed::testEngineSpeedNoOptimizations();

    state.restoreEngineState();
    testenginespeed::testEngineSpeedWithOptimizations();
    
    state.restoreEngineState();
    testenginespeed::testBfsSpeed();

    state.restoreEngineState();
    testenginespeed::testMctsSpeed();

    state.restoreEngineState();
    testrng::testStateAfterSimulations();
}

