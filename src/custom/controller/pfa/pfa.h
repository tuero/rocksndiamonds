

#ifndef PFA_H
#define PFA_H

#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <map>
#include <cmath>

#include "../base_controller.h"

#include "abstract_graph.h"
#include "abstract_node.h"

#include "../../engine/engine_types.h"
#include "../../engine/engine_helper.h"
#include "../../engine/action.h"
#include "../../util/timer.h"
#include "../../util/summary_window.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


class PFA : public BaseController {
private:
    Timer timer;
    AbstractGraph abstract_graph;
    std::deque<AbstractNode*> abstract_path;

    void logPath();

    void findPath(int abstract_level);

public:

    PFA();

    void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) override;

    void run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) override;    

};

#endif  //PFA_H


