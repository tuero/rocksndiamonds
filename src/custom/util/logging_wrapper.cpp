/**
 * @file: logging_wrapper.cpp
 *
 * @brief: Interface for logging and replay file.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "logging_wrapper.h"

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <plog/Appenders/ConsoleAppender.h>

#include "../engine/engine_helper.h"
#include "../engine/engine_types.h"


// Directory locations
const std::string LOG_DIR = "./src/custom/logs/";
const std::string REPLAY_DIR = "./src/custom/replays/";
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


namespace logwrap {
    
    /*
     * Initialize loggers for file and std_out.
     *
     * @param logLevel Max log level
     * @param programArgs String of all program args
     */
    void initLogger(plog::Severity logLevel, std::string &programArgs) {
        // log file name with path
        std::string logFileFullPath = LOG_DIR + getFileName() + LOG_EXTENSION;

        // Default logger to file
        plog::init<FileLogger>(logLevel, logFileFullPath.c_str());

        // Second logger to stdout
        static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
        plog::init<ConsolLogger>(logLevel, &consoleAppender);

        // Log CLAs used
        PLOGI_(logwrap::FileLogger) << programArgs;
    }


    /*
     * Initialize the replay file.
     *
     * This will hold all information needed for an exact replay
     * Includes levelset, level played, engine random number seed, and all actions
     * taken by the agent.
     */
    void initReplayFile() {
        // Don't create replay file if we are currently in a replay
        if (enginehelper::getControllerType() == enginetype::REPLAY) {
            return;
        }
        std::string replayFileFullPath = REPLAY_DIR + getFileName() + REPLAY_EXTENSION;
        PLOGI_(logwrap::FileLogger) << "Creating replay file " << replayFileFullPath;
        replayFile.open(replayFileFullPath, std::ios::app);
        saveReplayLevelInfo();
    }


    /*
     * Set max log level used for both loggers.
     *
     * @param logLevel The max level to log.
     */
    void setLogLevel(plog::Severity logLevel) {
        plog::get<logwrap::FileLogger>()->setMaxSeverity(logLevel);
        plog::get<logwrap::ConsolLogger>()->setMaxSeverity(logLevel);
    }


    /*
     * Log RNG seed, levelset and level number used.
     */
    void saveReplayLevelInfo() {
        if (!replayFile.is_open()) {
            PLOGE_(logwrap::FileLogger) << "Can't save replay level info, file already closed.";
            return;
        }

        replayFile << RNG::getEngineSeed() << std::endl;
        replayFile << enginehelper::getLevelSet() << std::endl;
        replayFile << enginehelper::getLevelNumber() << std::endl;
    }


    /*
     * Logs the engine type being used my the simulator.
     *
     * Depending on the level set being used, different parts of the simulator
     * are used. In most cases (and in all custom maps), TYPE_RND is used.
     */
    void logEngineType() {
        std::string msg;

        msg = "Using engine type ";
        if (level.game_engine_type == GAME_ENGINE_TYPE_EM) {
            msg += "EM";
        }
        else if (level.game_engine_type == GAME_ENGINE_TYPE_UNKNOWN) {
            msg += "UNKNOWN";
        }
        else if (level.game_engine_type == GAME_ENGINE_TYPE_RND) {
            msg += "RND";
        }

        PLOGI_(logwrap::FileLogger) << msg;
    }


    /*
     * Logs some of the important player fields.
     */
    void logPlayerDetails() {
        std::string msg = "Player Info: ";
        msg += "X: " + std::to_string(stored_player[0].jx) + " ";
        msg += "Y: " + std::to_string(stored_player[0].jy) + " ";
        msg += "MovPos: " + std::to_string(stored_player[0].MovPos) + " ";
        msg += "MovDir: " + std::to_string(stored_player[0].MovDir) + " ";
        msg += "is_moving: " + std::to_string(stored_player[0].is_moving) + " ";
        msg += "is_waiting: " + std::to_string(stored_player[0].is_waiting) + " ";
        msg += "move_delay: " + std::to_string(stored_player[0].move_delay) + " ";
        PLOGD_(logwrap::FileLogger) << msg;
    }


    /*
     * Logs the current board state (FELD) at the tile level.
     */
    void logBoardState() {
        std::string msg = "Board state at step: " + std::to_string(step_counter) + "\n";
        msg += getStructRepresentation(Feld);
        PLOGD_(logwrap::FileLogger) << msg;
    }


    /*
     * Logs the current board state (MovPos) sprite tile distance offsets.
     */
    void logMovPosState() {
        std::string msg = "MovPos at step: " + std::to_string(step_counter) + "\n";
        msg += getStructRepresentation(MovPos);
        PLOGD_(logwrap::FileLogger) << msg;
    }


    /*
     * Logs the current board state (MovDir) sprite tile direction offsets.
     */
    void logMovDirState() {
        std::string msg = "MovDir at step: " + std::to_string(step_counter) + "\n";
        msg += getStructRepresentation(MovDir);
        PLOGD_(logwrap::FileLogger) << msg;
    }


    /*
     * Logs the current tile distances to goal (used in pathfinding).
     */
    void logBoardDistances() {
        std::string msg = "Board distances:\n";
        msg += getStructRepresentation(enginehelper::distances);
        PLOGD_(logwrap::FileLogger) << msg;
    }


    /*
     * Log all information at level start.
     *
     * Includes engine type, player position and state, board item positions, and distances.
     */
    void logLevelStart() {
        PLOGD_(logwrap::FileLogger) << "Level start";
        logEngineType();
        logPlayerDetails();
        logBoardState();
        logBoardDistances();
    }


    /*
     * Log all information for the current state.
     *
     * Includes player position and state, board item positions, and directions.
     */
    void logState() {
        PLOGD_(logwrap::FileLogger) << "Current state";
        logPlayerDetails();
        logBoardState();
        logMovPosState();
        logMovDirState();
    }


    /*
     * Save the players current move to replay file.
     */
    void savePlayerMove(std::string &action) {
        // Don't record action if we are already in a replay
        if (options.controller_type == CONTROLLER_TYPE_REPLAY || !replayFile.is_open()) {
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

} //namespace logwrap