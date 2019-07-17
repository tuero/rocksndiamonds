
#include "custom.h"

#include <iostream>
#include <string>
#include <vector>
#include <ctime> 

#include "engine/game_state.h"
#include "engine/action.h"
#include "engine/engine_helper.h"
#include "engine/engine_types.h"
#include "controller/controller.h"
// Tests
#include "tests/test_engine_speed.h"
#include "tests/test_rng.h"
#include "util/logging_wrapper.h"
#include <plog/Log.h>

#include "util/rng.h"

// External variables accessible to engine
boolean is_simulating;

// Global objects which are needed outside local scopes
std::vector<Action> solution;
int solution_counter;
Controller controller;

// Level info
char *level_bd = (char *)"classic_boulderdash";
char *level_em = (char *)"classic_emerald_mine";
char *level_custom = (char *)"tuero";

std::string SEP(30, '-');



// ------------------------ Init Functions --------------------------

/*
 * Called whenever level is started
 * Calculates tile distances to goal for MCTS with goal, initializes Zorbrist Tables
 * for state hashing, and sends starting state information to logger.
 */
extern "C" void handleLevelStart() {
    // Calculate tile distances to goal
    enginehelper::dijkstra();

    // Initialize zorbrist tables for state hashing
    enginehelper::initZorbristTables();

    // Send starting board positions and player into to loggers
    debugEngineType();
    debugPlayerDetails();
    debugBoardState();
    debugBoardDistances();

    // Set controller and clear solution
    controller.setController(enginehelper::getControllerType());
    controller.clearSolution();

    // RNG seeding
    RNG::setInitialRandomBit();
}


/*
 * Initialize the loggers, as well as max log level
 * Two types of loggers: consol and file
 */
extern "C" void initLogger() {
    logwrap::initLogger((options.debug==TRUE ? plog::verbose : plog::error));
}


/*
 * Set the levelset. 
 * This should almost always be 0, as this corresponds to the users custom folder.
 * Higher values can be used if the level folders are copied to the user directory.
 */
extern "C" void setLevelSet(int levelset) {
    char *levelset_dir;

    // setup.c -> getHomeDir
    leveldir_current->basepath = (char *)"/home/tuero/.rocksndiamonds/levels";

    // Levelset subfolder names
    switch(levelset) {
        case 1: {levelset_dir = level_bd; break;}           // Boulder Dash
        case 2: {levelset_dir = level_em; break;}           // Emerald Mines
        default: {levelset_dir = level_custom; break;}      // Custom
    }

    PLOGI_(logwrap::FileLogger) << "Setting levelset " << levelset_dir;

    leveldir_current->fullpath = levelset_dir;
    leveldir_current->subdir = levelset_dir;
    leveldir_current->identifier = levelset_dir;
} 


// ----------------------- Action Handler --------------------------

/*
 * Get an action from the controller and send back to engine.
 * Implementation of solution will depend on controller type.
 */
extern "C" int getAction() {
    return controller.getAction();
}


// ----------------------------- RNG ------------------------------

/*
 * Wrapper to get random number for engine use
 * Engine already provides RNG, but this allows for reproducibility
 * during simulation.
 */
extern "C" int getRandomNumber(int max) {
    return RNG::getRandomNumber(max);
}

/*
 * Sets the random number generator seed
 * Reseeding is used for simulations, as the next state outcomes will
 * be different depending on whether or not simulations are used (as it
 * advances the RNG used by the engine)
 */
extern "C" void setRandomNumberSeed() {
    RNG::setSeedEngineHash();
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
    enginehelper::dijkstra();
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
    enginehelper::dijkstra();
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
    state.setFromSimulator();

    enginehelper::dijkstra();

    PLOGI_(logwrap::FileLogger) << msg;

    state.restoreSimulator();
    testenginespeed::testEngineSpeedNoOptimizations();

    state.restoreSimulator();
    testenginespeed::testEngineSpeedWithOptimizations();
    
    state.restoreSimulator();
    testenginespeed::testBfsSpeed();

    state.restoreSimulator();
    testenginespeed::testMctsSpeed();

    state.restoreSimulator();
    testrng::testStateAfterSimulations();
}


/*
 * Logs the engine type being used my the simulator
 * Depending on the level set being used, different parts of the simulator
 * are used. In most cases (and in all custom maps), TYPE_RND is used.
 */
extern "C" void debugEngineType() {
    logwrap::logEngineType();
}


/*
 * Logs some of the important player fields
 */
extern "C" void debugPlayerDetails() {
    logwrap::logPlayerDetails();
}


/*
 * Logs the current board state (FELD) at the tile level
 */
extern "C" void debugBoardState() {
    logwrap::logBoardState();
}


/*
 * Logs the current board state (MovPos) sprite tile distance offsets
 */
extern "C" void debugMovPosState() {
    logwrap::logMovPosState();
}


/*
 * Logs the current board state (MovDir) sprite tile direction offsets
 */
extern "C" void debugMovDirState() {
    logwrap::logMovDirState();
}


/*
 * Logs the current tile distances to goal (used in pathfinding)
 */
extern "C" void debugBoardDistances() {
   logwrap::logBoardDistances();
}
