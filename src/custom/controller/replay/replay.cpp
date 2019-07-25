

#include "replay.h"

Replay::Replay(){
    replayFileName = enginehelper::getReplayFileName();
    replayFileStream.open(replay_dir + replayFileName, std::ifstream::in);

    // First lines are level number and RNG seed
    int level_num;
    uint64_t seed;

    replayFileStream >> level_num;
    replayFileStream >> seed;

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

    // std::string replay_file = enginehelper::getReplayFileName();
    // setReplayFile(replay_file);
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
                currentSolution.push_back(stringToAction(line));
            }
            replayFileStream.close();
        }
    }
    catch (const std::ifstream::failure& e) {
        PLOGE_(logwrap::FileLogger) << "Cannot open file.";
    }

}
