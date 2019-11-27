/**
 * @file: logging_wrapper.cpp
 *
 * @brief: Interface for logging and replay file.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "logger.h"

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>

// Third party logging library
#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>

#include "../engine/engine_types.h"
#include "rng.h"


// Directory locations
const std::string LOG_DIR = "./src/ai/logs/";
const std::string REPLAY_DIR = "./src/ai/replays/";
const std::string STATS_DIR = "./src/ai/stats/";
const std::string LOG_EXTENSION = ".log";
const std::string REPLAY_EXTENSION = ".txt";
static std::ofstream replayFile;



/*
 * Get the current datetime in string format to name the log/replay files
 */
std::string datetimeToString() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    // TODO: Prone to overflow?
    strftime(buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",timeinfo);

    return std::string(buffer);
}


/*
 * Get the file name used for both logging and replay.
 * File name is the datetime and PID.
 */
std::string getFileName() {
    pid_t pid = getpid();
    return datetimeToString() + " " + std::to_string(pid);
}


/*
 * Helper function to get pretty print of internal engine data structure
 */
std::string getStructRepresentation(short (&data)[MAX_LEV_FIELDX][MAX_LEV_FIELDY]) {
    std::ostringstream os;
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            os.width(5);
            if (stored_player[0].jx == x && stored_player[0].jy == y && data[x][y] == 0) {
                os << "X ";
            }
            else {
                os << data[x][y] << " ";
            }
            
        }
        os << std::endl;
    }
    return os.str();
}


/*
 * Helper function to get pretty print of internal engine data structure
 */
std::string getStructRepresentation(int (&data)[MAX_LEV_FIELDX][MAX_LEV_FIELDY]) {
    std::ostringstream os;
    for (int y = 0; y < level.fieldy; y++) {
        for (int x = 0; x < level.fieldx; x++) {
            os.width(5);
            if (stored_player[0].jx == x && stored_player[0].jy == y && data[x][y] == 0) {
                os << " X ";
            }
            else {
                os << data[x][y] << " ";
            }
            
        }
        os << std::endl;
    }
    return os.str();
}


namespace logger {
    
    /*
     * Initialize loggers for file and std_out.
     *
     * @param logLevel Max log level
     * @param programArgs String of all program args
     */
    void initLogger(LogLevel logLevel, std::string &programArgs) {
        plog::Severity logLevel_ = static_cast<plog::Severity>(logLevel);
        
        // Set log level depending if debug flag set or if we are in a replay
        if (options.controller_type == CONTROLLER_REPLAY) {logLevel_ = plog::error;}

        // log file name with path
        std::string logFileFullPath = LOG_DIR + getFileName() + LOG_EXTENSION;

        // Default logger to file
        plog::init<FileLogger>(logLevel_, logFileFullPath.c_str());

        // Second logger to stdout
        static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
        plog::init<ConsoleLogger>(logLevel_, &consoleAppender);

        // Log CLAs used
        PLOGI_(logger::FileLogger) << programArgs;
    }


    /*
     * Initialize the replay file.
     *
     * This will hold all information needed for an exact replay
     * Includes levelset, level played, engine random number seed, and all actions
     * taken by the agent.
     */
    void initReplayFile(const std::string levelSet, int levelNumber) {
        std::string replayFileFullPath = REPLAY_DIR + getFileName() + REPLAY_EXTENSION;
        PLOGI_(logger::FileLogger) << "Creating replay file \"" << replayFileFullPath << "\"";
        replayFile.open(replayFileFullPath, std::ios::app);
        
        if (!replayFile.is_open()) {
            PLOGE_(logger::FileLogger) << "Can't save replay level info, file already closed.";
            return;
        }

        replayFile << RNG::getEngineSeed() << std::endl;
        replayFile << levelSet << std::endl;
        replayFile << levelNumber << std::endl;
    }


    /*
     * Set max log level used for both loggers.
     *
     * @param logLevel The max level to log.
     */
    void setLogLevel(LogLevel logLevel) {
        plog::Severity logLevel_ = static_cast<plog::Severity>(logLevel);
        plog::get<logger::FileLogger>()->setMaxSeverity(logLevel_);
        plog::get<logger::ConsoleLogger>()->setMaxSeverity(logLevel_);
    }


    /*
     * Logs the engine type being used my the simulator.
     *
     * Depending on the level set being used, different parts of the simulator
     * are used. In most cases (and in all custom maps), TYPE_RND is used.
     */
    void logEngineType() {
        std::string msg = "Using engine type ";
        if (level.game_engine_type == GAME_ENGINE_TYPE_EM) {
            msg += "EM";
        }
        else if (level.game_engine_type == GAME_ENGINE_TYPE_UNKNOWN) {
            msg += "UNKNOWN";
        }
        else if (level.game_engine_type == GAME_ENGINE_TYPE_RND) {
            msg += "RND";
        }

        PLOGI_(logger::FileLogger) << msg;
    }


    /*
     * Logs some of the important player fields.
     */
    void logPlayerDetails(plog::Severity logLevel) {
        std::string msg = "Player Info: ";
        msg += "X: " + std::to_string(stored_player[0].jx) + " ";
        msg += "Y: " + std::to_string(stored_player[0].jy) + " ";
        msg += "MovPos: " + std::to_string(stored_player[0].MovPos) + " ";
        msg += "MovDir: " + std::to_string(stored_player[0].MovDir) + " ";
        msg += "is_moving: " + std::to_string(stored_player[0].is_moving) + " ";
        msg += "is_waiting: " + std::to_string(stored_player[0].is_waiting) + " ";
        msg += "move_delay: " + std::to_string(stored_player[0].move_delay) + " ";
        PLOG_(logger::FileLogger, logLevel) << msg;
    }


    /*
     * Logs the current board state (FELD) at the tile level.
     */
    void logBoardState(plog::Severity logLevel) {
        PLOG_(logger::FileLogger, logLevel) << "Board state at step: " << step_counter << "\n" << getStructRepresentation(Feld);
    }


    /*
     * Logs the current board state (MovPos) sprite tile distance offsets.
     */
    void logMovPosState(plog::Severity logLevel) {
        PLOG_(logger::FileLogger, logLevel) << "MovPos at step: " << step_counter << "\n" << getStructRepresentation(MovPos);
    }


    /*
     * Logs the current board state (MovDir) sprite tile direction offsets.
     */
    void logMovDirState(plog::Severity logLevel) {
        PLOG_(logger::FileLogger, logLevel) << "MovDir at step: " << step_counter << "\n" << getStructRepresentation(MovDir);
    }


    /*
     * Logs the current tile distances to goal (used in pathfinding).
     */
    void logBoardDistances(plog::Severity logLevel) {
        PLOG_(logger::FileLogger, logLevel) << "Board distances:\n" << getStructRepresentation(distances);
    }


    /*
     * Logs the sprite IDs
     */
    void logBoardSpriteIDs(plog::Severity logLevel) {
        PLOG_(logger::FileLogger, logLevel) << "Sprite IDs:\n" << getStructRepresentation(spriteIDs);
    }


    /*
     * Log all information for the current state.
     *
     * Includes player position and state, board item positions, and directions.
     */
    void logCurrentState(plog::Severity logLevel) {
        PLOG_(logger::FileLogger, logLevel) << "Current state:";
        logPlayerDetails(logLevel);
        logBoardState(logLevel);
        logBoardSpriteIDs(logLevel);
        logMovPosState(logLevel);
        logMovDirState(logLevel);
    }


    /*
     * Log the players current move.
     */
    void logPlayerMove(const std::string &action) {
        PLOGD_(logger::FileLogger) << "Controller sending action: " + action;
        PLOGD_(logger::ConsoleLogger) << "Controller sending action: " + action;
    }


    /*
     * Save the players current move to replay file.
     */
    void savePlayerMove(const std::string &action) {
        // Don't record action if we are already in a replay
        if (options.controller_type == CONTROLLER_REPLAY || !replayFile.is_open()) {
            return;
        }

        replayFile << action << std::endl;
    }


    /*
     * Close the replay file for cleanup.
     */
    void closeReplayFile() {
        if (!replayFile.is_open()) {
            return;
        }
        replayFile.close();
    }

} //namespace logger