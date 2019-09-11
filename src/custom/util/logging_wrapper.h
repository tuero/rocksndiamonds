/**
 * @file: logging_wrapper.h
 *
 * @brief: Interface for logging and replay file.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef LOGGER_WRAPPER_H
#define LOGGER_WRAPPER_H

#include <string>
#include <plog/Log.h>


extern "C" {
    #include "../../main.h"
}


namespace logwrap {

    enum {FileLogger, ConsolLogger};

    /*
     * Initialize loggers for file and std_out
     */
    void initLogger(plog::Severity logLevel, std::string &programArgs);

    /*
     * Initialize the replay file.
     */
    void initReplayFile();

    /*
     * Set max log level used for both loggers
     */
    void setLogLevel(plog::Severity logLevel);

    /*
     * Log RNG seed, levelset and level number used.
     */
    void saveReplayLevelInfo();

    /*
     * Logs the engine type being used my the simulator
     * Depending on the level set being used, different parts of the simulator
     * are used. In most cases (and in all custom maps), TYPE_RND is used.
     */
    void logEngineType();

    /*
     * Logs some of the important player fields.
     */
    void logPlayerDetails();

    /*
     * Logs the current board state (FELD) at the tile level.
     */
    void logBoardState();

    /*
     * Logs the current board state (MovPos) sprite tile distance offsets.
     */
    void logMovPosState();

    /*
     * Logs the current board state (MovDir) sprite tile direction offsets.
     */
    void logMovDirState();

    /*
     * Logs the current tile distances to goal (used in pathfinding).
     */
    void logBoardDistances();

    /*
     * Log all information at level start
     * Includes engine type, player position and state, board item positions, and distances.
     */
    void logLevelStart();

    /*
     * Logs the current tile distances to goal (used in pathfinding).
     */
    void logState();

    /*
     * Save the players current move to replay file.
     */
    void savePlayerMove(std::string &action);

    /*
     * Close the replay file for cleanup.
     */
    void closeReplayFile();

}



#endif  //LOGGER_WRAPPER_H


