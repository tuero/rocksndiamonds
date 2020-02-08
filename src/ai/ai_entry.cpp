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

// Standard Libary/STL
#include <string>
#include <vector>

// Includes
#include "engine_helper.h"      // Engine helper functions
#include "rng.h"                // RNG
#include "logger.h"             // Logger
#include "statistics.h"

// Controller
#include "controller/controller.h"

// Misc
#include "level_programming/level_programming.h"

// Tests
#ifdef RUN_TESTS
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#endif


// External variables accessible to engine
// Used disable screenbuffer writing while not in headless mode
// (speeds up engine simulation)
boolean is_simulating;

// Global objects which are needed outside local scopes
Controller controller;

// Flag to check whether trying to load the level failed
boolean load_level_failed;


// ------------------------ Init Functions --------------------------

/**
 * Perform all necessary actions at level start.
 */
extern "C" void handleFirstLevelStart() {
    // If level is a custom programmed level, the respective level start code is called.
    levelprogramming::customLevelProgrammingStart();

    // Initialize zorbrist tables for state hashing
    enginehelper::initZorbristTables();

    // // Initialize sprite IDs
    enginehelper::initSpriteIDs();

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
    // if (enginehelper::getControllerType() != CONTROLLER_REPLAY) {
    if (options.save_run) {
        logger::initReplayFile(enginehelper::getLevelSet(), enginehelper::getLevelNumber());
    }

    controller.handleFirstLevelStart();
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


/**
 * Initalizes the controller to be used.
 * The controller type is provided by a command line argument, which the 
 * engine helper grabs.
 */
extern "C" void initController() {
    controller.initController();
}


/**
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


/**
 * Initialize the replay directory.
 */
extern "C" void initReplayDirectory(void) {
    logger::initReplayDirectory();
}


/**
 * Initialize the stats file.
 */
extern "C" void initStatsFile(void) {
    statistics::openStatsFile();
}


/**
 * Close the replay file.
 */
extern "C" void closeReplayFile(void) {
    logger::closeReplayFile();
}


/**
 * Close the stats file.
 */
extern "C" void closeStatsFile(void) {
    statistics::closeStatsFile();
}


/**
 * Output necessary statistics to file.
 */
extern "C" void outputStatsToFile(void) {
    statistics::outputStatsToFile();
}


/**
 * Set the levelset given by the command line argument.
 */
extern "C" void setLevelSet(void) {
    enginehelper::setLevelSet();
} 


/**
 * Load the level.
 */
extern "C" void loadLevel(int levelNumber) {
    enginehelper::loadLevel(levelNumber);
}

// ----------------------- Action Handler --------------------------

/**
 * Check if the controller wants to request a reset.
 */
extern "C" int requestReset() {
    return controller.requestReset();
}


/**
 * Get an action from the controller and send back to engine.
 * Implementation of solution will depend on controller type.
 */
extern "C" int getAction() {
    return controller.getAction();
}


/**
 * Some custom levels have elements that continuously spawn in
 * Hook needs to be made in event loop, as these features are not supported
 * in the built in CE programming
 */
extern "C" void handleCustomLevelProgramming() {
    levelprogramming::customLevelProgrammingUpdate();
}


// ----------------------------- RNG ------------------------------

/**
 * Wrapper to get random number for engine use.
 */
extern "C" int getRandomNumber(int max) {
    return RNG::getRandomNumber(max);
}



// ------------------------ Tests ------------------------------

#ifdef RUN_TESTS
/*
 * Runs all of the above tests in sequence
 * Results are logged to file.
 */
extern "C" int runTests() {
    int result = Catch::Session().run();
    return result;
}
#endif

