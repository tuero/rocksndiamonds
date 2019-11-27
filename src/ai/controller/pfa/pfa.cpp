
#include "pfa.h"


PFA::PFA() {}


void PFA::logPath() {
    std::string msg;
    PLOGI_(logger::FileLogger) << "Logging path: ";

    for (auto const &node: abstract_path) {
        PLOGI_(logger::FileLogger) << "Node id: " << node->getId();
    }
}


void PFA::findPath(int abstract_level) {
    timer.reset();
    timer.start();

    // Get start node
    AbstractNode* start_node = abstract_graph.getStartNode(abstract_level);
    std::vector<AbstractNode*> nodes = abstract_graph.getNodesAtLevel(abstract_level);

    for (auto const & node : nodes) {
        node->setValueG(std::numeric_limits<float>::max());
    }

    PLOGI_(logger::FileLogger) << "Abstraction level: " << start_node->getLevel();
    PLOGI_(logger::FileLogger) << "Start node: " << start_node->getId();
    PLOGI_(logger::FileLogger) << "Starting A*";
    PLOGI_(logger::ConsolLogger) << "Starting A*";

    // A* datastructures
    std::priority_queue<AbstractNode*, std::vector<AbstractNode*>, CompareAbstractNode> open_queue;
    std::map<int, AbstractNode*> open_map;
    std::map<int, AbstractNode*> closed;

    // Starting node
    start_node->setPathParent(nullptr);
    start_node->setValueG(0);
    open_queue.push(start_node);
    open_map[start_node->getId()] = start_node;

    // A* pathfinding on abstract level
    while (!open_queue.empty()) {
        // Pull minimal f node
        AbstractNode* node = open_queue.top();
        open_queue.pop();
        open_map.erase(node->getId());


        // consistent heuristic node skip
        // auto search = closed.find(node->getId());
        // if (search != closed.end()) {continue;}

        closed[node->getId()] = node;

        // Found goal
        if (node->representsGoal()) {
            PLOGI_(logger::FileLogger) << "Path found.";
            abstract_path.clear();

            while(node) {
                abstract_path.push_front(node);
                node = node->getPathParent();
            }

            PLOGI_(logger::FileLogger) << "Path length: " << abstract_path.size();
            break;
        }

        // Expand children
        std::map<int, AbstractNode*> children = node->getNeighbours();
        for (auto child : children) {
            int child_id = child.first;
            AbstractNode* child_node = child.second;
            float new_g = std::numeric_limits<float>::max();

            // if (node->getValueG() < new_g) {
            //     // new_g = node->getValueG() + 1;
            //     new_g = node->getValueG() + std::abs(node->getValueH() - child_node->getValueH());
            // }

            // auto search_closed = closed.find(child_id);
            // if (search_closed != closed.end()) {
            //     if (child_node->getValueG() <= new_g) {continue;}
            //     closed.erase(child_node->getId());
            // }

            // auto search_open = open_map.find(child_id);
            // if (search_open != open_map.end()) {
            //     if (child_node->getValueG() <= new_g) {continue;}
            //     open_map.erase(child_node->getId());
            // }

            // // Set g and parent
            // child_node->setValueG(new_g);
            // child_node->setPathParent(node);

            // // Save children
            // open_queue.push(child_node);
            // open_map[child_node->getId()] = child_node;



            // ------new
            new_g = node->getValueG() + std::abs(node->getValueH() - child_node->getValueH());
            // new_g = node->getValueG() + 1;
            if (open_map.find(child_id) != open_map.end()) {
                if (child_node->getValueG() <= new_g) {continue;}
                child_node->setValueG(new_g);
                child_node->setPathParent(node);
            }
            else if (closed.find(child_id) != closed.end()) {
                if (child_node->getValueG() <= new_g) {continue;}
                child_node->setValueG(new_g);
                child_node->setPathParent(node);
                closed.erase(child_node->getId());

                open_queue.push(child_node);
                open_map[child_node->getId()] = child_node;
            }
            else {
                child_node->setValueG(new_g);
                child_node->setPathParent(node);
                open_queue.push(child_node);
                open_map[child_node->getId()] = child_node;
            }

        }
    }

    timer.stop();

    PLOGI_(logger::FileLogger) << "Path Time: " << timer.getDuration();
    PLOGI_(logger::ConsolLogger) << "Path Time: " << timer.getDuration();
    logPath();

}



void PFA::handleLevelStart() {
    past_goal_ = enginehelper::getCurrentGoalLocation();
    abstract_graph.init();
    abstract_graph.abstract();

    abstract_level = abstract_graph.getLevelUsed();

    PLOGI_(logger::FileLogger) << "Level being used: " << abstract_level;
    PLOGI_(logger::ConsolLogger) << "Level being used: " << abstract_level;
    abstract_graph.logGraphLevel(abstract_level);

    abstract_path.clear();
    findPath(abstract_level);
}


void PFA::sendAbstractPathToSummaryWindow() {
    grid_representation = abstract_graph.getAbstractRepresentation(abstract_level, true);
    enginetype::GridCell goal_location = enginehelper::getCurrentGoalLocation();

    for (auto const & node : abstract_path) {
        for (auto const & grid_cell : node->getRepresentedCells()) {
            grid_representation[grid_cell.x][grid_cell.y] = -3;
        }
    }

    if (abstract_path.size() > 1) {
        for (auto const & grid_cell : goal_abstract_node->getRepresentedCells()) {
            grid_representation[grid_cell.x][grid_cell.y] = -2;
        }
    }
    else {
        grid_representation[goal_location.x][goal_location.y] = -2;
    }

    for (auto const & grid_cell : current_abstract_node->getRepresentedCells()) {
        grid_representation[grid_cell.x][grid_cell.y] = -1;
    }

    summarywindow::updateGridRepresentation(grid_representation);
}



void PFA::setNodeFromPath() {
    enginetype::GridCell player_cell = enginehelper::getPlayerPosition();

    while (!abstract_path.empty()) {
        std::vector<enginetype::GridCell> rep_cells = abstract_path.front()->getRepresentedCells();

        // If player is not in this abstract node, then we must have already visited
        // Remove until we find where the player is in our path
        if (std::find(rep_cells.begin(), rep_cells.end(), player_cell) != rep_cells.end()) {
            rep_cells.erase(rep_cells.begin());
            break;
        }
        abstract_path.erase(abstract_path.begin());
    }

    current_abstract_node = abstract_path.front();
    goal_abstract_node = abstract_path.front();

    if (abstract_path.size() > 1) {
        goal_abstract_node = abstract_path[1];
    }
}


void PFA::handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) {
    // Go to state MCTS is currently in
    GameState reference_state;
    reference_state.setFromEngineState();
    enginehelper::setSimulatorFlag(true);
    for (unsigned int i = 0; i < forwardSolution.size(); i++) {
        enginehelper::setEnginePlayerAction(forwardSolution[i]);
        enginehelper::engineSimulateSingle();
    }

    // Check if we need to rerun A* on abstract path
    enginetype::GridCell current_goal = enginehelper::getCurrentGoalLocation();
    if (past_goal_.x != current_goal.x || past_goal_.y != current_goal.y) {
        abstract_graph.setGoal(current_goal);
        abstract_path.clear();
        findPath(abstract_level);
        abstract_graph.logGraphLevel(abstract_level);
    }
    past_goal_ = current_goal;

    // We are in the abstract node which contains the goal
    setNodeFromPath();


    // Restore back to current state
    reference_state.restoreEngineState();
    enginehelper::setSimulatorFlag(false);

    sendAbstractPathToSummaryWindow();

    // pfa_mcts.handleEmpty(currentSolution, forwardSolution, current_abstract_node, goal_abstract_node);
    pfa_mcts.handleEmpty(currentSolution, forwardSolution, abstract_path, goal_abstract_node);
}


void PFA::run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) 
{
    pfa_mcts.run(currentSolution, forwardSolution, statistics);
}