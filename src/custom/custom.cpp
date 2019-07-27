
#include "custom.h"

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <ctime> 
#include <random>

// engine
#include "engine/game_state.h"
#include "engine/action.h"
#include "engine/engine_helper.h"
#include "engine/engine_types.h"

// controller
#include "controller/controller.h"

// util and logging
#include "util/level_programming.h"
#include "util/rng.h"
#include "util/logging_wrapper.h"
#include <plog/Log.h>

// Tests
#include "tests/test_engine_speed.h"
#include "tests/test_rng.h"



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

std::string levelset_survival = "custom_survival";

std::string SEP(30, '-');



// ------------------------ Init Functions --------------------------

/*
 * Called whenever level is started
 * Calculates tile distances to goal for MCTS with goal, initializes Zorbrist Tables
 * for state hashing, and sends starting state information to logger.
 */
extern "C" void handleLevelStart() {
    // Log level
    PLOGI_(logwrap::FileLogger) << "Level being played: " << level.file_info.nr;

    // Calculate tile distances to goal
    int goal_x, goal_y;
    enginehelper::findGoalLocation(goal_x, goal_y);
    enginehelper::setBoardDistances(goal_x, goal_y);

    // Initialize zorbrist tables for state hashing
    enginehelper::initZorbristTables();

    // Send starting board positions and player into to loggers
    debugEngineType();
    debugPlayerDetails();
    debugBoardState();
    debugBoardDistances();

    // clear solution
    controller.clearSolution();

    // Ensure RNG seeds reset during level start
    RNG::setEngineSeed(RNG::getEngineSeed());
    RNG::setSimulatingSeed(RNG::getSimulationSeed());
}


/*
 * Initialize the controller to be used
 * Controller is supplied by a command line argument
 */
extern "C" void initController() {
    controller.setController(enginehelper::getControllerType());
}


/*
 * Initialize the loggers, as well as max log level
 * Two types of loggers: consol and file
 */
extern "C" void initLogger(int argc, char *argv[]) {
    std::vector<std::string> allArgs(argv, argv + argc);
    std::string cla_args;

    // Convert args vector to string
    for (unsigned int i = 0; i < allArgs.size(); i++) {
        cla_args += allArgs[i] + " ";
    }

    // Set log level depending if debug flag set or if we are in a replay
    plog::Severity log_level = plog::info;
    if (options.debug == TRUE) {log_level = plog::verbose;}
    if (options.controller_type == CONTROLLER_TYPE_REPLAY) {log_level = plog::error;}

    logwrap::initLogger(log_level, cla_args);
}


/*
 * Set the levelset given by the command line argument
 */
extern "C" void setLevelSet(void) {
    enginehelper::setLevelSet();
} 


/*
 * Save the RNG seed, levelset and level used
 */
extern "C" void saveReplayLevelInfo(void) {
    if (options.controller_type == CONTROLLER_TYPE_REPLAY) {return;}
    logwrap::saveReplayLevelInfo();
}


// ----------------------- Action Handler --------------------------

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
extern "C" void spawnElements() {
    std::string subdir_string(leveldir_current->subdir);
    if (subdir_string == levelset_survival) {
        levelprogramming::spawnElements();
    }
}


int yamCounter = 0;
int spaceCounter = 0;

// rowIndex, yamCounter, spaceCounter
std::vector<std::array<int, 3>> counters = {{6,0,0}, {10,0,0}};

void addElement(std::array<int, 3> &counters) {
    // create space
    if (counters[1] == 5) {
        counters[1] = 0;
        counters[2] = RNG::getRandomNumber(16) + 16;
        return;
    }

    // Spacing decrement
    if (counters[2] != 0) {
        counters[2] -= 1;
        return;
    }

    // Add new object
    Feld[0][counters[0]] = enginetype::FIELD_CUSTOM_12;
    MovDir[0][counters[0]] = 2;
    counters[1] += 1;
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
    // RNG::setSeedEngineHash();
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
    int goal_x, goal_y;
    enginehelper::findGoalLocation(goal_x, goal_y);
    enginehelper::setBoardDistances(goal_x, goal_y);
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
    int goal_x, goal_y;
    enginehelper::findGoalLocation(goal_x, goal_y);
    enginehelper::setBoardDistances(goal_x, goal_y);
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

    int goal_x, goal_y;
    enginehelper::findGoalLocation(goal_x, goal_y);
    enginehelper::setBoardDistances(goal_x, goal_y);

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
