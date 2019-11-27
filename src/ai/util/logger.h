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
#include "../engine/engine_types.h"

extern "C" {
    #include "../../main.h"
    #include "../../game.h"
}


namespace logger {

    enum {FileLogger, ConsoleLogger};
    enum class LogLevel {none, fatal, error, warning, info, debug, verbose};

    /*
     * Initialize loggers for file and std_out
     */
    void initLogger(LogLevel logLevel, std::string &programArgs);

    /*
     * Initialize the replay file.
     */
    void initReplayFile(const std::string levelSet, int levelNumber);

    /*
     * Set max log level used for both loggers
     */
    void setLogLevel(LogLevel logLevel);

    /*
     * Logs the engine type being used my the simulator
     * Depending on the level set being used, different parts of the simulator
     * are used. In most cases (and in all custom maps), TYPE_RND is used.
     */
    void logEngineType();

    /*
     * Logs some of the important player fields.
     */
    void logPlayerDetails(plog::Severity logLevel = plog::verbose);

    /*
     * Logs the current board state (FELD) at the tile level.
     */
    void logBoardState(plog::Severity logLevel = plog::verbose);

    /*
     * Logs the current board state (MovPos) sprite tile distance offsets.
     */
    void logMovPosState(plog::Severity logLevel = plog::verbose);

    /*
     * Logs the current board state (MovDir) sprite tile direction offsets.
     */
    void logMovDirState(plog::Severity logLevel = plog::verbose);

    /*
     * Logs the current tile distances to goal (used in pathfinding).
     */
    void logBoardDistances(plog::Severity logLevel = plog::verbose);

    /*
     * Logs the sprite IDs
     */
    void logBoardSpriteIDs(plog::Severity logLevel = plog::verbose);

    /*
     * Logs the current tile distances to goal (used in pathfinding).
     */
    void logCurrentState(plog::Severity logLevel = plog::verbose);

    /*
     * Log the players current move.
     */
    void logPlayerMove(const std::string &action);

    /*
     * Save the players current move to replay file.
     */
    void savePlayerMove(const std::string &action);

    /*
     * Close the replay file for cleanup.
     */
    void closeReplayFile();

}



#endif  //LOGGER_WRAPPER_H


