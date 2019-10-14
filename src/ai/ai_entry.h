/**
 * @file: ai_entry.h
 *
 * @brief: Exposed functions which are called in game engine
 * 
 * @author: Jake Tuero
 * Date: June 2019
 * Contact: tuero@ualberta.ca
 */


#ifndef CUSTOM_H
#define CUSTOM_H


#ifdef __cplusplus
extern "C"
{
#endif
    // ---------------- C Declarations ------------------
    #include "../main.h"
    #include "../screens.h"


    // ---------------- Init Functions ------------------

    /*
     * Perform all necessary actions at level start.
     *
     * Called whenever level is started. Calculates tile distances to goal for 
     * MCTS with goal, initializes Zorbrist Tables for state hashing, and sends 
     * starting state information to logger. Levels with custom programming call 
     * the respective level start actions.
     */
    void handleLevelStart(void);

    /*
     * Set the controller to be used.
     * 
     * The controller is supplied by the -controller command line argument. If none is 
     * given, then the default user controller is used (user keyboard input)
     */
    void setController(void);

    /*
     * Initialize the loggers.
     *
     * A file and consol logger are used. Consol logger only displays partial information
     * which would be of interest to the user.
     *
     * @param argc Number of command line arguments supplied to program
     * @param argc Char array of command line arguments supplied to program
     */
    void initLogger(int argc, char *argv[]);

    /*
     * Set the levelset given by the command line argument.
     */
    void setLevelSet(void); 

    /*
     * Save the RNG seed, levelset and level used.
     */
    void saveReplayLevelInfo(void);

    // ----------------------- Summary Window -------------------------

    /*
     * Close the summary window.
     */
    void closeMapWindow(void);

    // --------------- Action Handler ------------------

    /*
     * Get an action from the controller (implementation specific) and send back to engine.
     *
     * @return Integer representation of action as defined by the engine.
     */
    int getAction(void);

    /*
     * Call level specific actions for custom programming levels.
     * 
     * Some custom levels have elements that continuously spawn in
     * Hook needs to be made in event loop, as these features are not supported
     * in the built in CE programming
     */
    void handleCustomLevelProgramming(void);


    // -------------------- RNG ----------------------

    /*
     * Wrapper to get random number for engine use.
     * 
     * Engine already provides RNG, but this allows for reproducibility
     * during simulation.
     *
     * @return Next random number from the uniform generator.
     */
    int getRandomNumber(int max);


    // ---------------- Tests ----------------------

    /*
     * Test the engine simulator speed.
     * These tests will simulate random player actions and progress the environment
     * forward. Optimizations are blocked engine code that is not needed during simulation,
     * Such as drawing to screen buffers.
     * Results are logged to file
     */
    void testEngineSpeed(void);

    /*
     * Tests the speed of running BFS
     * Dijkstra must be ran first to get the tile distances to goal.
     * Results are logged to file
     */
    void testBFSSpeed(void);

    /*
     * Tests the speed of running MCTS
     * This does one sweep of MCTS with a budget of 20ms (as per engine spec of
     * having a 20ms frame delay).
     * Results are logged to file
     */
    void testMCTSSpeed(void);

    /*
     * Tests for RNG reproducibility after engine simulations during rollouts
     */
    void testRNGAfterSimulations(void);

    /*
     * Runs all of the above tests in sequence
     * Results are logged to file
     */
    void testAll(void);



#ifdef __cplusplus
}
#endif






#endif  //CUSTOM_H


