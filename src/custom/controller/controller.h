

#ifndef CONTROLLER_H
#define CONTROLLER_H


#include <vector>
#include <map>
#include <memory>
#include "../engine/action.h"           // Action enum
#include "../engine/engine_types.h"
#include "../engine/engine_helper.h" 
//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   

#include "base_controller.h"
#include "bfs/bfs.h"
#include "mcts/mcts.h"
#include "replay/replay.h"
#include "user/user.h"
#include "pfa/pfa.h"


class Controller {

private:
    int solution_counter;
    std::map<enginetype::Statistics, int> statistics = {{enginetype::RUN_TIME, 0},
                                                        {enginetype::COUNT_EXPANDED_NODES, 0}, 
                                                        {enginetype::COUNT_SIMULATED_NODES, 0},
                                                        {enginetype::MAX_DEPTH, 0}
                                                       };
    std::vector<Action> currentSolution;
    std::vector<Action> forwardSolution;
    std::unique_ptr<BaseController> baseController;

    unsigned int step_counter = 0;

public:
    Controller();

    Controller(enginetype::ControllerType controller);

    void clearSolution();

    Action getAction();

    unsigned int getRunTime();

    unsigned int getCountExpandedNodes();

    unsigned int getCountSimulatedNodes();

    unsigned int getMaxDepth();



    std::vector<Action> &getSolution();

    void setController(enginetype::ControllerType controller);

};



#endif  //CONTROLLER_H


