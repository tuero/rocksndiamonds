/**
 * @file: ai_entry.h
 *
 * @brief: Exposed functions which are called in game engine
 * 
 * @author: Jake Tuero
 * Date: June 2019
 * Contact: tuero@ualberta.ca
 */


#ifndef AI_ENTRY_H
#define AI_ENTRY_H


#ifdef __cplusplus
extern "C"
{
#endif

    // ---------------- Init Functions ------------------

    /**
     * Perform all necessary actions at first time the level is started.
     *
     * Called whenever level is started. Calculates tile distances to goal for 
     * MCTS with goal, initializes Zorbrist Tables for state hashing, and sends 
     * starting state information to logger. Levels with custom programming call 
     * the respective level start actions.
     */
    void handleFirstLevelStart(void);

    /**
     * Call controller action handler when level is solved. 
     * This will close logging and cleanup.
     */
    void handleLevelSolved(void);

    /**
     * Call controller action handler when level is failed. 
     * Depending on the controller, this will either terminate or attempt the level again.
     */
    void handleLevelFailed(void);

    /**
     * Initalizes the controller to be used.
     * 
     * The controller is supplied by the -controller command line argument. If none is 
     * given, then the default user controller is used (user keyboard input)
     */
    void initController(void);

    /**
     * Initialize the loggers.
     *
     * A file and consol logger are used. Consol logger only displays partial information
     * which would be of interest to the user.
     *
     * @param argc Number of command line arguments supplied to program
     * @param argc Char array of command line arguments supplied to program
     */
    void initLogger(int argc, char *argv[]);

    /**
     * Set the levelset given by the command line argument.
     */
    void setLevelSet(void); 

    /**
     * Set the level
     * 
     * @param levelNumber The level number for the respective levelset.
     */
    void setLevel(int levelNumber);


    // --------------- Action Handler ------------------

    /**
     * Check if the controller wants to request a reset.
     * 
     * @return True if the controller wants to request a rest.
     */
    int requestReset(void);

    /**
     * Get an action from the controller (implementation specific) and send back to engine.
     *
     * @return Integer representation of action as defined by the engine.
     */
    int getAction(void);

    /**
     * Call level specific actions for custom programming levels.
     * 
     * Some custom levels have elements that continuously spawn in
     * Hook needs to be made in event loop, as these features are not supported
     * in the built in CE programming
     */
    void handleCustomLevelProgramming(void);


    // -------------------- RNG ----------------------

    /**
     * Wrapper to get random number for engine use.
     * 
     * Engine already provides RNG, but this allows for reproducibility
     * during simulation.
     *
     * @return Next random number from the uniform generator.
     */
    int getRandomNumber(int max);


    // ---------------- Tests ----------------------

    #ifdef HEADLESS
    /**
     * Runs all of the above tests in sequence
     * Results are logged to file
     */
    int runTests(void);
    #endif



#ifdef __cplusplus
}
#endif



#endif  //AI_ENTRY_H


