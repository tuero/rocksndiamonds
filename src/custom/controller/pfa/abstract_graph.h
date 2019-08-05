

#ifndef ABSTRACTGRAPH_H
#define ABSTRACTGRAPH_H

#include <iostream>
#include <vector>
#include <deque>
#include <map>
#include <algorithm>
#include <memory>

#include "abstract_node.h"

#include "../../engine/engine_helper.h"
#include "../util/timer.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


class AbstractGraph {
    typedef std::unique_ptr<AbstractNode> Pointer;
private:
    // std::vector<Pointer> current_level_;          // current children from this node
    int level;
    std::map<int, Pointer> current_level_;
    std::map<int, Pointer> next_level_;
    std::map<int, Pointer>::iterator it;

    std::map<enginetype::GridCell, std::vector<enginetype::GridCell>> grid_neighbours_;

    void findCliquesThree();

    void findCliquesTwo();

    void findCliquesOne();

    void joinNeighbours();

    void setCurrentFromNext();

public:

    AbstractGraph(); 

    void abstract();

    void setGoal(int goal_x, int goal_y);

    void printGraph();


};


#endif  //ABSTRACTGRAPH_H


