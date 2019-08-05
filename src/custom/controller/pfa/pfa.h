

#ifndef MCTS_H
#define MCTS_H

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "../base_controller.h"

#include "abstract_graph.h"
#include "../../engine/engine_types.h"
#include "../../engine/engine_helper.h"
#include "../../engine/action.h"
#include "../../util/timer.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


class PFA : public BaseController {
private:
    Timer timer;
    AbstractGraph abstract_graph;

public:

    PFA();

    void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) override;

    void run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) override;    

};

#endif  //MCTS


