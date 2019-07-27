

#include "replay.h"

Replay::Replay(){
    replayFileName = enginehelper::getReplayFileName();
    replayFileStream.open(replay_dir + replayFileName, std::ifstream::in);

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
    enginehelper::setLevelSet();
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
