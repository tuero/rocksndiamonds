/**
 * @file: replay.h
 *
 * @brief: Controller for reading/replaying previous games.
 * 
 * @author: Jake Tuero
 * Date: October 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef REPLAY_H
#define REPLAY_H


// Standard Libary/STL
#include <iostream>
#include <fstream>
#include <string>

// Includes
#include "base_controller.h" 


/**
 * Controller to handle replay files.
 *
 * This reads and redirects every action as listed in the replay file.
 */
class Replay : public BaseController {
private:
    const std::string REPLAY_DIR = "./src/ai/replays/";         // Replay files location
    std::string replayFileName;                                 // Replay file name (directed through CLA)
    std::ifstream replayFileStream;                             // Replay file stream

public:

    Replay();

    /**
     * Set the replay file to use as a file stream.
     * 
     * @param file The replay file string name.
     */
    void setReplayFile(std::string &file);

    /**
     * Flag for controller to try again if level fails.
     * Replay files can handle replays.
     * 
     * @return True.
     */
    bool retryOnLevelFail() const override {return true;}

    /**
     * Get the action from the replay file.
     * If there is a reset command, the level is restarted.
     * 
     * @return The next action from the replay file.
     */
    Action getAction() override;

};




#endif  //REPLAY_H


