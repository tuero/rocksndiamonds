
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
     * Called whenever level is started
     * Calculates tile distances to goal for MCTS with goal, initializes Zorbrist Tables
     * for state hashing, and sends starting state information to logger.
     */
    void handleLevelStart(void);

    /*
     * Initialize the controller to be used
     * Controller is supplied by a command line argument
     */
    void initController(void);

    /*
     * Initialize the loggers, as well as max log level
     * Two types of loggers: consol and file
     */
    void initLogger(int argc, char *argv[]);

    /*
     * Set the levelset given by the command line argument
     */
    void setLevelSet(void); 

    /*
     * Save the RNG seed, levelset and level used
     */
    void saveReplayLevelInfo(void);

    // ----------------------- Summary Window -------------------------

    /*
     * Close the summary window.
     */
    void closeMapWindow(void);

    // --------------- Action Handler ------------------

    /*
     * Get an action from the controller and send back to engine.
     * Implementation of solution will depend on controller type.
     */
    int getAction(void);

    /*
     * Some custom levels have elements that continuously spawn in
     * Hook needs to be made in event loop, as these features are not supported
     * in the built in CE programming
     */
    void handleCustomLevelProgramming(void);


    // -------------------- RNG ----------------------

    /*
     * Wrapper to get random number for engine use
     * Engine already provides RNG, but this allows for reproducibility
     * during simulation.
     */
    int getRandomNumber(int max);

    /*
     * Sets the random number generator seed
     * Reseeding is used for simulations, as the next state outcomes will
     * be different depending on whether or not simulations are used (as it
     * advances the RNG used by the engine)
     */
    void setRandomNumberSeed(void);

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


    // ------------- Debug Logging ------------------

    /*
     * Logs the engine type being used my the simulator
     * Depending on the level set being used, different parts of the simulator
     * are used. In most cases (and in all custom maps), TYPE_RND is used.
     */
    void debugEngineType(void);

    /*
     * Logs some of the important player fields
     */
    void debugPlayerDetails(void);

    /*
     * Logs the current board state (FELD) at the tile level
     */
    void debugBoardState(void);

    /*
     * Logs the current board state (MovPos) sprite tile distance offsets
     */
    void debugMovPosState(void);

    /*
     * Logs the current board state (MovDir) sprite tile direction offsets
     */
    void debugMovDirState(void);

    /*
     * Logs the current tile distances to goal (used in pathfinding)
     */
    void debugBoardDistances(void);

#ifdef __cplusplus
}
#endif






#endif  //CUSTOM_H


