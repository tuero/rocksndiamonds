
#include "pfa.h"


PFA::PFA() {
}


void PFA::logPath() {
    std::string msg;
    PLOGI_(logwrap::FileLogger) << "Logging path: ";

    for (auto const &node: abstract_path) {
        // msg = "Node id: " << node->getId();
        PLOGI_(logwrap::FileLogger) << "Node id: " << node->getId();
    }
}


void PFA::findPath() {
    // Get start node
    // int abstract_level = abstract_graph.getLevel() / 2;
    // int abstract_level = log(abstract_graph.getLevel());
    int abstract_level = 4;
    enginetype::GridCell player_grid = enginehelper::getPlayerPosition();
    AbstractNode* start_node = abstract_graph.getStartNode(abstract_level);

    PLOGI_(logwrap::FileLogger) << "Abstraction level: " << start_node->getLevel();
    PLOGI_(logwrap::FileLogger) << "Start node: " << start_node->getId();
    PLOGI_(logwrap::FileLogger) << "Starting A*";

    // A* datastructures
    std::priority_queue<AbstractNode*, std::vector<AbstractNode*>, CompareAbstractNode> open_queue;
    std::map<int, AbstractNode*> open_map;
    std::map<int, AbstractNode*> closed;

    start_node->setPathParent(nullptr);
    open_queue.push(start_node);
    open_map[start_node->getId()] = start_node;

    // A*
    while (!open_queue.empty()) {
        // Pull minimal f node
        AbstractNode* node = open_queue.top();
        open_queue.pop();
        open_map.erase(node->getId());


        // consistent heuristic node skip
        auto search = closed.find(node->getId());
        if (search != closed.end()) {continue;}

        closed[node->getId()] = node;

        // Found goal
        if (node->representsGoal()) {
            PLOGI_(logwrap::FileLogger) << "Path found.";
            abstract_path.clear();

            while(node) {
                abstract_path.push_front(node);
                node = node->getPathParent();
            }

            PLOGI_(logwrap::FileLogger) << "Path length: " << abstract_path.size();
            return;
        }

        std::map<int, AbstractNode*> children = node->getNeighbours();
        for (auto child : children) {
            int child_id = child.first;
            AbstractNode* child_node = child.second;

            float new_g = node->getValueG() + 1;

            auto search_closed = closed.find(child_id);
            if (search_closed != closed.end()) {
                if (child_node->getValueG() <= new_g) {continue;}
                closed.erase(child_node->getId());
            }

            auto search_open = open_map.find(child_id);
            if (search_open != open_map.end()) {
                if (child_node->getValueG() <= new_g) {continue;}
                open_map.erase(child_node->getId());
            }

            child_node->setValueG(new_g);
            child_node->setPathParent(node);

            open_queue.push(child_node);
            open_map[child_node->getId()] = child_node;
        }
    }

}


void PFA::handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) {
    // Silent compiler warning
    // (void)currentSolution;
    // (void)forwardSolution;

    // Abstract graph
    abstract_graph.init();
    abstract_graph.abstract();

    // Find path on abstract graph
    abstract_path.clear();

    Timer timer;

    timer.start();
    findPath();
    timer.stop();

    PLOGI_(logwrap::FileLogger) << "Path Time: " << timer.getDuration();
    logPath();

    for (int i = 0; i < 1000; i++) {
        currentSolution.push_back(Action::noop);
        forwardSolution.push_back(Action::noop);
    }
}


void PFA::run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) 
{
    // Silent compiler warning
    (void)currentSolution;
    (void)forwardSolution;
    (void)statistics;
}