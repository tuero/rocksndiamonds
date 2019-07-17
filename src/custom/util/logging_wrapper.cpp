

#include "logging_wrapper.h"


namespace logwrap {


    /*
     * Get the current datetime in string format
     * Used for naming log files
     */
    std::string datetimeToString() {
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];

        time (&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);

        return std::string(buffer);
    }


    /*
     * Initialize loggers 
     * Loggers for stdout and log file
     */
    void initLogger(plog::Severity log_level) {
        // log file name with path
        std::string log_file_path = log_dir + datetimeToString() + log_suffix;

        // Don't create replay file if we are currently in a replay
        if (enginehelper::getControllerType() != enginetype::REPLAY) {
            saved_run_file.open(log_dir + datetimeToString() + run_suffix, std::ios::app);
        }

        // Default logger to file
        // Will log all severity levels
        plog::init<FileLogger>(log_level, log_file_path.c_str());

        // Second logger to stdout
        // This will log depending on CLA
        static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
        plog::init<ConsolLogger>(log_level, &consoleAppender);

        // Log CLA used
        
    }


    /*
     * Set max log level used for both loggers
     */
    void setLogLevel(plog::Severity log_level) {
        plog::get<logwrap::FileLogger>()->setMaxSeverity(log_level);
        plog::get<logwrap::ConsolLogger>()->setMaxSeverity(log_level);
    }


    /*
     * Logs the engine type being used my the simulator
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

        PLOGD_(logwrap::FileLogger) << msg;
    }


    /*
     * Logs some of the important player fields
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
     * Helper function to get pretty print of internal engine data structure
     */
    std::string _getStructRepresentation(short (&data)[MAX_LEV_FIELDX][MAX_LEV_FIELDY]) {
        std::ostringstream os;
        for (int y = 0; y < level.fieldy; y++) {
            for (int x = 0; x < level.fieldx; x++) {
                os.width(5);
                if (stored_player[0].jx == x and stored_player[0].jy == y && data[x][y] == 0) {
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
     * Logs the current board state (FELD) at the tile level
     */
    void logBoardState() {
        std::string msg = "Board state at step: " + std::to_string(step_counter) + "\n";
        msg += _getStructRepresentation(Feld);
        PLOGD_(logwrap::FileLogger) << msg;
    }


    /*
     * Logs the current board state (MovPos) sprite tile distance offsets
     */
    void logMovPosState() {
        std::string msg = "MovPos at step: " + std::to_string(step_counter) + "\n";
        msg += _getStructRepresentation(MovPos);
        PLOGD_(logwrap::FileLogger) << msg;
    }


    /*
     * Logs the current board state (MovDir) sprite tile direction offsets
     */
    void logMovDirState() {
        std::string msg = "MovDir at step: " + std::to_string(step_counter) + "\n";
        msg += _getStructRepresentation(MovDir);
        PLOGD_(logwrap::FileLogger) << msg;
    }


    /*
     * Logs the current tile distances to goal (used in pathfinding)
     */
    void logBoardDistances() {
        std::string msg = "Board distances:\n";
        msg += _getStructRepresentation(enginehelper::distances);
        PLOGD_(logwrap::FileLogger) << msg;
    }


    /*
     * Save the players current move.
     * Used for replays
     */
    void savePlayerMove(std::string &action) {
        if (!saved_run_file.is_open()) {
            PLOGE_(logwrap::FileLogger) << "Can't save player move, file is already closed.";
            return;
        }
        saved_run_file << action << std::endl;
    }


    /*
     * Close the replay file for cleanup.
     */
    void closeSaveRunFile() {
        if (!saved_run_file.is_open()) {
            return;
        }
        saved_run_file.close();
    }

}