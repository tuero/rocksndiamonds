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
    // Game 
    #include "../../main.h"
    #include "../../game.h"
}


namespace logger {

    enum {FileLogger, ConsoleLogger};
    enum class LogLevel {none, fatal, error, warning, info, debug, verbose};

    /**
     * Initialize loggers for file and std_out.
     * 
     * @param logLevel Max log level
     * @param programArgs String of all program args
     */
    void initLogger(LogLevel logLevel, std::string &programArgs);

    /**
     * Initialize the replay file.
     * 
     * @param levelSet The string name of the levelSet (for loading during replay).
     * @param levelNumber The level number being played.
     */
    void initReplayFile(const std::string levelSet, int levelNumber);

    /**
     * Set max log level used for both loggers.
     * 
     * @param logLevel The max level to log.
     */
    void setLogLevel(LogLevel logLevel);

    /**
     * Logs the engine type being used my the simulator
     * Depending on the level set being used, different parts of the simulator
     * are used. In most cases (and in all custom maps), TYPE_RND is used.
     */
    void logEngineType();

    /**
     * Logs some of the important player fields.
     * 
     * @param logLevel The loglevel to log the player details.
     */
    void logPlayerDetails(plog::Severity logLevel = plog::verbose);

    /**
     * Logs the current board state (FELD) at the tile level.
     * 
     * @param logLevel The loglevel to log the board state.
     */
    void logBoardState(plog::Severity logLevel = plog::verbose);

    /**
     * Logs the current board state (MovPos) sprite tile distance offsets.
     * 
     * @param logLevel The loglevel to log the board element board offsets.
     */
    void logMovPosState(plog::Severity logLevel = plog::verbose);

    /**
     * Logs the current board state (MovDir) sprite tile direction offsets.
     * 
     * @param logLevel The loglevel to log the board element directions.
     */
    void logMovDirState(plog::Severity logLevel = plog::verbose);

    /**
     * Logs the current tile distances to goal (used in pathfinding).
     * 
     * @param logLevel The loglevel to log the board distances
     */
    void logBoardDistances(plog::Severity logLevel = plog::verbose);

    /**
     * Logs the sprite IDs.
     * 
     * @param logLevel The loglevel to log the board sprite IDs
     */
    void logBoardSpriteIDs(plog::Severity logLevel = plog::verbose);

    /**
     * Logs the current state.
     * This include player, board state, board offsets, directions, and sprite IDs.
     * 
     * @param logLevel The loglevel to log the current state information.
     */
    void logCurrentState(plog::Severity logLevel = plog::verbose);

    /**
     * Log the players current move.
     * 
     * @param action The action taken in string format.
     */
    void logPlayerMove(const std::string &action);

    /**
     * Save the players current move to replay file.
     * 
     * @param action The action taken in string format.
     */
    void savePlayerMove(const std::string &action);

    /**
     * Close the replay file for cleanup.
     */
    void closeReplayFile();

}



#endif  //LOGGER_WRAPPER_H


