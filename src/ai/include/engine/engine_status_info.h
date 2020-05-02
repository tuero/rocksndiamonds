/**
 * @file: enigne_status_info.h
 *
 * @brief: Interface for controllers to access engine status information from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


// Standard Library/STL
#include <string>

// Includes
#include "controller_listing.h"
#include "engine_types.h"

namespace enginehelper {
namespace enginestate {

    /**
     * Get the controller type defined by user command line argument.
     * 
     * The type of available controllers are defined by system.h by enum controller_type,
     * which is set by the command line argument -controller <CONTROLLER_NAME>.
     *
     * @return Contoller enum type.
     */
    ControllerType getControllerType();

    /**
     * Get the replay file name.
     *
     * The replay file name is supplied by the command line argument -replay <REPLAY_FILE>.
     * 
     * @return Replay file name string.
     */
    std::string getReplayFileName();

    /**
     * Get the optional CLA for general use.
     * 
     * @return Integer optional CLA.
     */
    int getOptParam();

    /**
     * Get the model path CLA
     * 
     * @return String relative path for the model to use
     */
    std::string getModelPathParam();

    /**
     * Check if the current status of the engine is loss of life.
     * The internal engine checkGameFailed() function is called. 
     *
     * @return True if the current status of the engine is failed.
     */
    bool engineGameFailed();

    /**
     * Check if the current status of the engine is level solved.
     * Structs game.LevelSolved and game.LevelSolved_GameEnd need to be checked.
     *
     * @return True if the current status of the engine is solved.
     */
    bool engineGameSolved();

    /**
     * Check if the current status of the engine is failed or solved
     *
     * @return True if the current status of engine is failed or solved.
     */
    bool engineGameOver();

    /**
     * Set the game engine mode status to GAME_MODE_QUIT.
     * This will trigger the program to end after cleanup. 
     */
    void setEngineGameStatusModeQuit();

    /**
     * Set the game engine mode status to GAME_MODE_PLAYING.
     * This will allow the game loop to query actions from controller. 
     */
    void setEngineGameStatusModePlaying();

    /**
     * Get the rate (number of engine ticks) for which it takes the agent to make a move.
     */
    int getEngineUpdateRate();

    /**
     * Check if the player is currently in the middle of executing an action.
     * 
     * @return True if the player is in the middle of grid cells (moving)
     */
    bool isPlayerDoneAction();

    /**
     * Set the action for the engine to perform on behalf of the player on the next iteration.
     *
     * The default agent (in single player mode) is in the struct stored_player[0]. The engine
     * looks at the action member each tick and performs the stored action.
     *
     * @param action The action to perform (may be noop)
     */
    void setEnginePlayerAction(Action action);

    /**
     * Set the stored player's action as a valid random action.
     *
     * To allow for fast simulations, this doesn't check whether the action is
     * useful i.e. not moving left if there is a wall to the left of the player.
     */
    void setEngineRandomPlayerAction();

    /**
     * Get the currently stored player action.
     *
     * @return The Action set for the stored player.
     */
    Action getEnginePlayerAction();

    /**
     * Get the current score of the game.
     *
     * @return The score of the game.
     */
    int getCurrentScore();

    /**
     * Get the Time left in the game.
     */
    int getTimeLeftScore();

    /**
     * Simulate the engine ahead a single tick.
     *
     * This calls the HandleGameActions() engine function, which performs one game tick.
     * The players action needs to be set before calling this function. If the player is
     * currently in motion, its action is continued.
     */
    void engineSimulateSingle();

    /**
     * Simulate the engine ahead enginestate::getEngineUpdateRate()=8 ticks.
     *
     * At normal speed, objects take 8 game ticks to take an action (move completely to a 
     * neighbouring grid cell). Objects take multiples or fractions of enginestate::getEngineUpdateRate()
     * if they have a faster/slower movement speed, but the player always takes 
     * enginestate::getEngineUpdateRate() to perform and complete an action.
     *
     * This calls the HandleGameActions() engine function enginestate::getEngineUpdateRate() times. 
     * The players action needs to be set before calling this function. 
     */
    void engineSimulate();

    /**
     * Set flag for simulating.
     *
     * This should be set to True whenever the controller wants to simulate, and 
     * set back to false before returning the action back to the calling function.
     * By default, this will be returned back to false by the calling function.
     * 
     * This will cause blocking actions in engine such as not rending to screen.
     * Profiling shows a 10x in speed with simulator_flag set, which is important when
     * wanting to do many simulations such as in MCTS.
     *
     * @param simulator_flag
     */
    void setSimulatorFlag(bool simulatorFlag);

    /**
     * Get the simulator flag status.
     *
     * @param True if the simulator flag is currently set.
     */
    bool isSimulating();

} //enginestate
} //enginehelper