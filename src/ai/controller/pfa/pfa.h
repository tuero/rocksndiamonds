

#ifndef PFA_H
#define PFA_H

#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <map>
#include <cmath>

#include "abstract_graph.h"
#include "abstract_node.h"
#include "pfa_mcts.h"

#include "../base_controller.h"

#include "../../engine/engine_types.h"
#include "../../engine/engine_helper.h"
#include "../../engine/action.h"
#include "../../util/timer.h"
#include "../../util/summary_window.h"

//Logging
#include "../../util/logging_wrapper.h"
#include <plog/Log.h>   


class PFA : public BaseController {
private:
    int abstract_level;
    Timer timer;
    AbstractGraph abstract_graph;
    PFA_MCTS pfa_mcts;
    std::deque<AbstractNode*> abstract_path;
    enginetype::GridCell past_goal_;

    std::vector<std::vector<int>> grid_representation;
    AbstractNode* current_abstract_node;
    AbstractNode* goal_abstract_node;

    void logPath();

    void findPath(int abstract_level);

    void sendAbstractPathToSummaryWindow();

    void setNodeFromPath();

public:

    PFA();

    void handleLevelStart() override;

    void handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) override;

    void run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) override;    

};

#endif  //PFA_H


