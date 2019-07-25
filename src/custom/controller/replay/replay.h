

#ifndef REPLAY_H
#define REPLAY_H


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <time.h>

#include "../base_controller.h"

#include "../../engine/game_state.h"
#include "../../engine/engine_types.h"
#include "../../engine/engine_helper.h"
#include "../../engine/action.h"
#include "../../util/timer.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


class Replay : public BaseController {
private:
    const std::string replay_dir = "./src/custom/logs/";
    std::string replayFileName;
    std::ifstream replayFileStream;

public:

    Replay();

    void setReplayFile(std::string &file);

    void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) override;

    void run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) override;    

};




#endif  //REPLAY_H


