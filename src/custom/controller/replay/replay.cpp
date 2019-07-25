

#include "replay.h"

Replay::Replay(){
    replayFileName = enginehelper::getReplayFileName();
    replayFileStream.open(replay_dir + replayFileName, std::ifstream::in);

    // First lines are level number and RNG seed
    int level_num;
    uint64_t seed;

    replayFileStream >> seed;
    replayFileStream >> level_num;

    RNG::setEngineSeed(seed);
    enginehelper::loadLevel(level_num);
}


void Replay::setReplayFile(std::string &file) {
    replayFileName = file;
}


void Replay::handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) {
    // Silent compiler warning
    (void)currentSolution;
    (void)forwardSolution;
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
        // std::ifstream replayFile (replay_dir + replayFileName);
        std::vector<Action> path;
        if (replayFileStream.is_open()){
            while (std::getline(replayFileStream,line)){
                if (line.length() == 0) {continue;}
                currentSolution.push_back(stringToAction(line));
            }
            replayFileStream.close();
        }
    }
    catch (const std::ifstream::failure& e) {
        PLOGE_(logwrap::FileLogger) << "Cannot open file.";
    }

}
