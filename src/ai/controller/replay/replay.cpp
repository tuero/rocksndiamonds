/**
 * @file: replay.cpp
 *
 * @brief: Controller for reading/replaying previous games.
 * 
 * @author: Jake Tuero
 * Date: October 2019
 * Contact: tuero@ualberta.ca
 */

#include "replay.h"

// Includes
#include "rng.h"
#include "engine_types.h"
#include "engine_helper.h"
#include "logger.h"


/**
 * Default constructor.
 * 
 * Initial read of RNG seed, levelset and level number. Then calls engine to 
 * set the levelset/level for playing.
 */
Replay::Replay(){
    try{
        requestReset_ = false;
        replayFileName = enginehelper::getReplayFileName();
        replayFileStream.open(REPLAY_DIR + replayFileName, std::ifstream::in);

        uint64_t seed;
        std::string level_set;
        int level_num;

        // File order is seed, levelset, level number
        // Reset of file are actions taken
        replayFileStream >> seed;
        replayFileStream >> level_set;
        replayFileStream >> level_num;

        // Call respective methods to set above data
        RNG::setEngineSeed(seed);
        options.level_set = (char*)level_set.c_str();
        enginehelper::setLevelSet(true);
        enginehelper::loadLevel(level_num);
    }
    catch (...) {
        PLOGE_(logger::ConsoleLogger) << "An error occured trying to initialize levelset/level from replay file. Exiting.";
        PLOGE_(logger::FileLogger) << "An error occured trying to initialize levelset/level from replay file. Exiting.";
        enginehelper::setEngineGameStatusModeQuit();
    }
}


/**
 * Set the replay file to use as a file stream.
 */
void Replay::setReplayFile(std::string &file) {
    replayFileName = file;
}


/**
 * Get the action from the replay file.
 * If there is a reset command, the level is restarted.
 */
Action Replay::getAction() {
    std::string line;
    requestReset_ = false;
    try{
        if (replayFileStream.is_open()){
            if (std::getline(replayFileStream,line) && line.length() != 0) {
                if (line == "reset") {
                    requestReset_ = true;
                    return Action::noop;
                }
                return enginehelper::stringToAction(line);
            }
        }
    }
    catch (const std::ifstream::failure& e) {
        PLOGE_(logger::FileLogger) << "Cannot open file.";
    }
    
    return Action::noop;
}

