

#ifndef ABSTRACTGRAPH_H
#define ABSTRACTGRAPH_H

#include <iostream>
#include <vector>
#include <array>
#include <deque>
#include <map>
#include <set>
#include <algorithm>
#include <memory>
#include <limits>
#include <fstream>

#include "abstract_node.h"

#include "../../engine/engine_helper.h"
#include "../util/timer.h"

//Logging
#include "../util/logging_wrapper.h"
#include <plog/Log.h>   


class AbstractGraph {
    typedef std::unique_ptr<AbstractNode> Pointer;
private:
    int id_;
    int level_;
    std::map<int, Pointer> current_level_;
    std::map<int, Pointer> next_level_;
    std::set<int> nodes_to_delete;
    // std::map<int, Pointer>::iterator it;


    std::map<enginetype::GridCell, std::vector<enginetype::GridCell>> grid_neighbours_;

    void findCliquesThree2();
    void findCliquesThree();

    void findCliquesTwo();

    void findCliquesOne();

    void joinNeighbours();

    void setCurrentFromNext();

    void calcDistancesRecursive(AbstractNode* current_node);

    void boardPrint(std::vector<std::vector<int>> &print_array);

public:

    AbstractGraph(); 

    void init();

    void abstract();

    AbstractNode* getTopLevelNode();

    AbstractNode* getStartNode(int level);

    void setGoal(int goal_x, int goal_y);

    void logGraph();

    int getLevel();


};


#endif  //ABSTRACTGRAPH_H


