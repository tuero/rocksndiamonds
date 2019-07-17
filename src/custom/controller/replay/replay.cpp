

#include "replay.h"

Replay::Replay(){}


void Replay::setReplayFile(std::string &file) {
    replayFileName = file;
}


void Replay::handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) {
    // Silent compiler warning
    (void)currentSolution;
    (void)forwardSolution;

    std::string replay_file = enginehelper::getReplayFileName();
    setReplayFile(replay_file);
}


void Replay::run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) 
{
    // Silent compiler warning
    (void)forwardSolution;
    (void)statistics;

    // We already have a solution
    if (!currentSolution.empty()) {
        return;
    }

    std::string line;
    try{
        std::ifstream replayFile (replay_dir + replayFileName);
        std::vector<Action> path;
        if (replayFile.is_open()){
            while (std::getline(replayFile,line)){
                currentSolution.push_back(stringToAction(line));
            }
            replayFile.close();
        }
    }
    catch (const std::ifstream::failure& e) {
        PLOGE_(logwrap::FileLogger) << "Cannot open file.";
    }

}
