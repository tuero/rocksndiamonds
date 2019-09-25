

#include "replay.h"

Replay::Replay(){
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
    enginehelper::setLevelSet();
    enginehelper::loadLevel(level_num);
}


void Replay::setReplayFile(std::string &file) {
    replayFileName = file;
}


void Replay::handleEmpty(BaseOption **currentOption, BaseOption **nextOption) {
    // Silent compiler warning
    (void)currentOption;
    (void)nextOption;

    *currentOption = *nextOption;
    // abstract_graph.init();
    // abstract_graph.abstract();
    // int abstract_level = abstract_graph.getLevelUsed();
    // grid_representation = abstract_graph.getAbstractRepresentation(abstract_level, true);
    // summarywindow::updateGridRepresentation(grid_representation);
}


void Replay::run(BaseOption **currentOption, BaseOption **nextOption, 
        std::map<enginetype::Statistics, int> &statistics) 
{
    // Silent compiler warning
    (void)currentOption;
    (void)nextOption;
    (void)statistics;

    // We already have a solution
    // if (!currentSolution.empty()) {
    //     return;
    // }

    std::string line;
    std::string optionName = "Single step action: ";
    try{
        std::vector<Action> path;
        if (replayFileStream.is_open()){
            // while (std::getline(replayFileStream,line)){
            //     if (line.length() == 0) {continue;}
            //     currentSolution.push_back(stringToAction(line));
            // }
            // replayFileStream.close();
            if (std::getline(replayFileStream,line) && line.length() != 0) {
                Action action = stringToAction(line);
                for (BaseOption *option : availableOptions_) {
                    if ((option->optionToString()).compare(optionName + actionToString(action)) == 0) {
                        *nextOption = option;
                        break;
                    }
                }
            }
        }
    }
    catch (const std::ifstream::failure& e) {
        PLOGE_(logwrap::FileLogger) << "Cannot open file.";
    }

}
