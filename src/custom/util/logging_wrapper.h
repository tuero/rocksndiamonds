

#ifndef LOGGER_WRAPPER_H
#define LOGGER_WRAPPER_H

#include <iostream>
#include <string>
#include <fstream>
#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>

#include "../engine/engine_helper.h"
#include "../engine/engine_types.h"


extern "C" {
    #include "../../main.h"
}


namespace logwrap {

    enum {FileLogger, ConsolLogger};

    static const std::string log_dir = "./src/custom/logs/";
    static const std::string log_suffix = ".log";
    static const std::string run_suffix = ".txt";
    static std::ofstream saved_run_file;


    /*
     * Get the current datetime in string format
     * Used for naming log files
     */
    std::string datetimeToString();

    /*
     * Initialize loggers 
     * Loggers for stdout and log file
     */
    void initLogger(plog::Severity log_level);

    /*
     * Set max log level used for both loggers
     */
    void setLogLevel(plog::Severity log_level);

    /*
     * Logs the engine type being used my the simulator
     * Depending on the level set being used, different parts of the simulator
     * are used. In most cases (and in all custom maps), TYPE_RND is used.
     */
    void logEngineType();

    /*
     * Logs some of the important player fields
     */
    void logPlayerDetails();

    /*
     * Logs the current board state (FELD) at the tile level
     */
    void logBoardState();

    /*
     * Logs the current board state (MovPos) sprite tile distance offsets
     */
    void logMovPosState();

    /*
     * Logs the current board state (MovDir) sprite tile direction offsets
     */
    void logMovDirState();

    /*
     * Logs the current tile distances to goal (used in pathfinding)
     */
    void logBoardDistances();

    /*
     * Save the players current move.
     * Used for replays
     */
    void savePlayerMove(std::string &action);

    /*
     * Close the replay file for cleanup.
     */
    void closeSaveRunFile();

}



#endif  //LOGGER_WRAPPER_H


