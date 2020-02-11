/**
 * @file: enigne_status_info.cpp
 *
 * @brief: Implementation of engine status information from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include "engine_helper.h"

// Standard Libary/STL
#include <random>

// Includes
#include "engine_types.h"       // GridCell, Action
#include "controller_listing.h"
#include "logger.h"

// Game headers
extern "C" {
    #include "../../game.h"                 // checkGameFailed, game
    #include "../../main.h"                 // stored_player
    #include "../../screens.h"              // HandleGameActions
    #include "../../libgame/system.h"       // options
}


namespace enginehelper {
namespace enginestate {

/**
 * Get the controller type defined by user command line argument.
 */
ControllerType getControllerType() {
    return options.controller_type;
}

/**
 * Get the replay file name.
 */
std::string getReplayFileName() {
    std::string replay_file(options.replay_file);
    return replay_file;
}


/**
 * Get the option CLA for general use.
 */
int getOptParam() {
    return options.opt;
}


/**
 * Check if the current status of the engine is loss of life.
 */
bool engineGameFailed() {
    return checkGameFailed();
}


/**
 * Check if the current status of the engine is level solved.
 */
bool engineGameSolved() {
    return (game.LevelSolved && !game.LevelSolved_GameEnd);
}


/**
 * Check if the current status of the engine is failed or solved
 */
bool engineGameOver() {
    return engineGameFailed() || engineGameSolved();
}


/**
 * Set the game engine mode status to GAME_MODE_QUIT.
 * This will trigger the program to end after cleanup. 
 */
void setEngineGameStatusModeQuit() {
    game_status = GAME_MODE_QUIT;
}

/**
 * Set the game engine mode status to GAME_MODE_PLAYING.
 * This will allow the game loop to query actions from controller. 
 */
void setEngineGameStatusModePlaying() {
    game_status = GAME_MODE_PLAYING;
}


/**
 * Check if the player is currently in the middle of executing an action.
 */
bool isPlayerDoneAction() {
    return stored_player[0].MovPos == 0;
}


/**
 * Set the action for the engine to perform on behalf of the player on the next iteration.
 */
void setEnginePlayerAction(Action action) {
    stored_player[0].action = action;
}


/**
 * Set the stored player's action as a valid random action.
 */
void setEngineRandomPlayerAction() {
    static std::random_device rand_dev;
    static std::mt19937 gen(rand_dev());
    static std::uniform_int_distribution<int> dist(0, enginetype::NUM_MV-1);

    int action = dist(gen);
    stored_player[0].action = (action == enginetype::NUM_MV-1) ? 0 : (1 << action);
}


/**
 * Get the currently stored player action.
 */
Action getEnginePlayerAction() {
    return static_cast<Action>(stored_player[0].action);
}


/**
 * Get the current score of the game.
 */
int getCurrentScore() {
    return game.score;
}


/**
 * Get the Time left in the game.
 */
int getTimeLeftScore() {
    return TimeLeft;
}


/**
 * Simulate the engine ahead a single tick.
 */
void engineSimulateSingle() {
    is_simulating = true;
    HandleGameActions();
    is_simulating = false;
}


/**
 * Simulate the engine ahead ENGINE_RESOLUTION game ticks.
 */
void engineSimulate() {
    is_simulating = true;
    for (int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        HandleGameActions();
    }
    is_simulating = false;
}

/**
 * Set flag for simulating.
 */
void setSimulatorFlag(bool simulatorFlag) {
    is_simulating = simulatorFlag;
}


/**
 * Get the simulator flag status.
 */
bool isSimulating() {
    return is_simulating;
}

} //namespace enginestate
} //namespace enginehelper