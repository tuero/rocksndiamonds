

#include "abstract_graph.h"


void AbstractGraph::init() {
    enginehelper::setBoardDistancesL1();

    // Populate base level with node representing each grid
    int width = enginehelper::getLevelWidth();
    int height = enginehelper::getLevelHeight();
    level_ = 0;
    id_ = 0;

    Timer timer;
    timer.start();

    // Create a node for each map grid tile
    for (int y = 0; y < width; y++) {
        for (int x = 0; x < height; x++) {
            float h = (float)enginehelper::distances[x][y];
            int id = next_level_.size();

            // Distance of -1 indicates a stationary wall
            if (h == -1) {
                continue;
            }

            // Create and store node
            enginetype::GridCell grid_cell = {x,y};
            Pointer node = std::make_unique<AbstractNode>(id_, level_, h, grid_cell);
            next_level_[id_] = std::move(node);
            id_ += 1;
        }
    }

    // All nodes are created, now we can start joining neighbours
    joinNeighbours();

    // Set current_level_ to next_level_ 
    setCurrentFromNext();

    timer.stop();

    PLOGI_(logwrap::FileLogger) << "Init Time: " << timer.getDuration();

}


AbstractGraph::AbstractGraph() {}


AbstractNode* AbstractGraph::getTopLevelNode() {
    return current_level_.begin()->second.get();
    // return flat_nodes_[0].begin()->second.get();
}


AbstractNode* AbstractGraph::getStartNode(int level) {
    if (level < 0 || level > level_) {
        PLOGE_(logwrap::FileLogger) << "Invalid level.";
        return nullptr;
    }

    // Top level node and grid the player is in
    AbstractNode* start_node = getTopLevelNode();
    enginetype::GridCell player_grid = enginehelper::getPlayerPosition();

    // Traverse down the abstractions until we reach desired level
    while(start_node->getLevel() > level) {
        std::vector<AbstractNode*> children =  start_node->getChildren();
        // Find the child which has the grid we care about
        for (auto child : children) {
            if (child->representsCell(player_grid)) {
                start_node = child;
                break;
            }
        }
    }

    return start_node;
}


void AbstractGraph::findCliquesThree2() {
    for (auto it = current_level_.begin(); it != current_level_.end(); ++it) {
        int current_id = it->first;
        std::map<int, AbstractNode*> current_neighbours = it->second.get()->getNeighbours();
        bool removed = false;

        // Node not in a clique of 2 if it has fewer then 2 neighbours
        if (current_neighbours.size() < 2) {++it; continue;}

        for (auto neighbour : current_neighbours) {
            if (neighbour.first <= current_id) {continue;}
            auto temp_search1 = current_level_.find(neighbour.first);
            if (temp_search1 == current_level_.end()) {continue;}

            std::map<int, AbstractNode*> second_neighbours = neighbour.second->getNeighbours();

            for (auto second_neighbour : second_neighbours) {
                if (second_neighbour.first == current_id) {continue;}
                if (second_neighbour.first <= current_id || second_neighbour.first < neighbour.first) {continue;}
                auto temp_search2 = current_level_.find(second_neighbour.first);
                if (temp_search2 == current_level_.end()) {continue;}

                std::map<int, AbstractNode*> third_neighbours = second_neighbour.second->getNeighbours();

                // Found clique
                auto search = third_neighbours.find(current_id);
                if (search != third_neighbours.end()) {
                    std::cout << "Found CLique " << current_id << " " << neighbour.first << " " << second_neighbour.first << std::endl;
                    std::vector<Pointer> children;
                    children.push_back(std::move(it->second));
                    children.push_back(std::move(temp_search2->second));
                    children.push_back(std::move(temp_search1->second));

                    // Make node and store
                    Pointer node = std::make_unique<AbstractNode>(id_, level_, children);
                    next_level_[id_] = std::move(node);
                    id_ += 1;

                    // Erase clique from current level
                    current_level_.erase(neighbour.first);
                    current_level_.erase(second_neighbour.first);
                    it = current_level_.erase(it);
                    removed = true;
                    break;
                }
            }
            if (removed) {break;}
        }

        if (!removed) {++it;}
    }
}


void AbstractGraph::findCliquesThree() {
    for (auto it = current_level_.begin(); it != current_level_.end(); ) {
        bool removed = false;
        int current_id = it->first;
        std::map<int, AbstractNode*> current_neighbours = it->second.get()->getNeighbours();

        // Node not in a clique of 2 if it has fewer then 2 neighbours
        if (current_neighbours.size() < 2) {++it; continue;}

        // Check for clique
        // Simplified version of Chiba and Nishizeki [1985] as we only need to find a single clique
        // and maximal clique size is 3
        // If neighbour of current_node also has a neighbour in current_neighbours then we have 
        // a 3-clique
        for (auto const &neighbour : current_neighbours) {
            int neighbour_id = neighbour.first;
            AbstractNode* second_node = neighbour.second;
            std::map<int, AbstractNode*> second_neighbours = second_node->getNeighbours();

            // If neighbour_id is less than current_neighbours, then it was already considered
            if (neighbour_id <= current_id) {continue;}

            // If neighbour is not in current_level_, then it has been previously used
            // in another clique and so we skip to another neighbour
            auto search = current_level_.find(neighbour_id);
            if (search == current_level_.end()) {continue;}

            // Loop until we find 3rd node for clique
            for (auto const &second_neighbour : second_neighbours) {
                int second_neighbour_id = second_neighbour.first;

                // Again, if 2nd neighbour id < current_neighbours, then it was already considered
                if (second_neighbour_id <= current_id || second_neighbour_id <= neighbour_id) {continue;}

                // If the 2nd neighbour id in current_neighbours, then we found a clique
                // We don't need this iterator later, so we use temp
                auto temp_search = current_neighbours.find(second_neighbour_id);
                if (temp_search == current_neighbours.end()) {continue;}

                // Sanity check that node wasn't previously used in clique, and save
                // search result to remove
                auto search_second = current_level_.find(second_neighbour_id);
                if (search_second == current_level_.end()) {continue;}

                // Otherwise, neighbours are free to be used in clique
                std::vector<Pointer> children;
                children.push_back(std::move(it->second));
                children.push_back(std::move(search->second));
                children.push_back(std::move(search_second->second));

                // Make node and store
                int id = next_level_.size();
                Pointer node = std::make_unique<AbstractNode>(id_, level_, children);
                next_level_[id_] = std::move(node);
                id_ += 1;

                // Erase clique from current level
                current_level_.erase(second_neighbour_id);
                current_level_.erase(neighbour_id);
                it = current_level_.erase(it);
                removed = true;
                break;
            }
            if (removed) {break;}

        }

        // If we removed then we do not increment iterator
        if (!removed) {++it;}
    }
}


void AbstractGraph::findCliquesTwo() {
    for (auto it = current_level_.begin(); it != current_level_.end(); ) {
        bool removed = false;
        std::map<int, AbstractNode*> current_neighbours = it->second.get()->getNeighbours();

        // Node not in a clique of size 2 if it has fewer than 1 neighbours
        if (current_neighbours.size() < 1) {continue;}

        // Check for clique of size 2
        float min_value = std::numeric_limits<float>::max();
        AbstractNode* saved_neighbour = nullptr;

        for (auto const &neighbour : current_neighbours) {
            int neighbour_id = neighbour.first;

            // If neighbour is not in current_level_, then it has been previously used
            // in another clique and so we skip to another neighbour
            auto search = current_level_.find(neighbour_id);
            if (search == current_level_.end()) {continue;}





            // Otherwise, neighbour is free to be used in clique
            float difference = std::abs(neighbour.second->getValueH() - it->second.get()->getValueH());
            if (difference < min_value) {
                removed = true;
                min_value = difference;
                saved_neighbour = neighbour.second;
            }

            // std::vector<Pointer> children;
            // children.push_back(std::move(it->second));
            // children.push_back(std::move(search->second));

            // // Make node and store
            // int id = next_level_.size();
            // Pointer node = std::make_unique<AbstractNode>(id_, level_, children);
            // next_level_[id_] = std::move(node);
            // id_ += 1;

            // // Erase clique from current level
            // current_level_.erase(neighbour_id);
            // it = current_level_.erase(it);
            // removed = true;
            // break;
        }

        if (removed) {
            auto search2 = current_level_.find(saved_neighbour->getId());
            std::vector<Pointer> children;
            children.push_back(std::move(it->second));
            children.push_back(std::move(search2->second));

            // Make node and store
            int id = next_level_.size();
            Pointer node = std::make_unique<AbstractNode>(id_, level_, children);
            next_level_[id_] = std::move(node);
            id_ += 1;

            // Erase clique from current level
            current_level_.erase(saved_neighbour->getId());
            it = current_level_.erase(it);
        }

        // If we removed then we do not increment iterator
        if (!removed) {++it;}
    }
}


void AbstractGraph::findCliquesOne() {
    // Remaining nodes were not a part of larger size cliques
    // so we abstract themselves
    for (auto it = current_level_.begin(); it != current_level_.end(); ++it) {
        int id = next_level_.size();

        // Don't use initialization list as it uses copy-constructor
        // and breaks references
        std::vector<Pointer> children;
        children.push_back(std::move(it->second));

        // Make node and store
        Pointer node = std::make_unique<AbstractNode>(id_, level_, children);
        next_level_[id_] = std::move(node);
        id_ += 1;
    }
}


void AbstractGraph::joinNeighbours() {
    for (auto it = next_level_.begin(); it != next_level_.end(); ++it) {
            for (auto it_inner = std::next(it); it_inner != next_level_.end(); ++it_inner) {
                AbstractNode* node_left = it->second.get();
                AbstractNode* node_right = it_inner->second.get();

                std::vector<enginetype::GridCell> cells_left = node_left->getRepresentedCells();
                std::vector<enginetype::GridCell> cells_right = node_right->getRepresentedCells();

                if (enginehelper::checkIfNeighbours(cells_left, cells_right)) {
                    node_left->addNeighbour(node_right);
                    node_right->addNeighbour(node_left);
                }
            }
        }
}

void AbstractGraph::setCurrentFromNext() {
    current_level_.clear();
    for (auto it = next_level_.begin(); it != next_level_.end(); ++it) {
        current_level_[it->first] = std::move(it->second);
    }
    next_level_.clear();
}


void AbstractGraph::calcDistancesRecursive(AbstractNode* current_node) {
    // Base level, value is fetched from distances struct
    if (current_node->getLevel() == 0) {
        std::vector<enginetype::GridCell> represented_cells = current_node->getRepresentedCells();
        enginetype::GridCell grid_cell = represented_cells[0];
        float value = (float)enginehelper::distances[grid_cell.x][grid_cell.y];
        current_node->setValueH(value);
        return;
    }

    // Update all children
    std::vector<AbstractNode*> children = current_node->getChildren();
    for (auto child : children) {
        calcDistancesRecursive(child);
    }

    // Children have updated value, so recompute current
    current_node->updateValue();
}

void AbstractGraph::setGoal(int goal_x, int goal_y) {
    // Run L1 to set distances
    enginehelper::setBoardDistancesL1(goal_x, goal_y);

    // Update nodes in graph
    // current_level_ should the single abstracted node 
    for (auto const &node : current_level_) {
        calcDistancesRecursive(node.second.get());
    }
}


void AbstractGraph::boardPrint(std::vector<std::vector<int>> &print_array){
    std::ostringstream os;
    os << std::endl;

    int width = enginehelper::getLevelWidth();
    int height = enginehelper::getLevelHeight();

    enginetype::GridCell player_pos = enginehelper::getPlayerPosition();
    enginetype::GridCell goal_pos = enginehelper::getCurrentGoalLocation();

    int min_value = print_array[0][0];
    for (int y = 0; y < width; y++) {
        for (int x = 0; x < height; x++) {
            if (print_array[y][x] != -1 && print_array[y][x] < min_value) {min_value = print_array[y][x];}            
        }
    }

    LOGD_(logwrap::FileLogger) << "Min value used: " << min_value;

    for (int y = 0; y < width; y++) {
        for (int x = 0; x < height; x++) {
            os.width(5);
            if (x == player_pos.x && y == player_pos.y) {os << "P" << " ";}
            else if (x == goal_pos.x && y == goal_pos.y) {os << "G" << " ";}
            else if (print_array[y][x] == -1) {
                os << "." << " ";
            }
            else {
                os << print_array[y][x] - min_value << " ";
            }
            
        }
        os << std::endl;
    }
    LOGD_(logwrap::FileLogger) << os.str();
}


void AbstractGraph::logGraph() {
    std::deque<AbstractNode*> current_level;
    std::string header = "----------- Abstract Graph Display -----------";
    std::string msg;

    int width = enginehelper::getLevelWidth();
    int height = enginehelper::getLevelHeight();

    std::vector<std::vector<int>> print_array;

    // Create a node for each map grid tile
    for (int y = 0; y < width; y++) {
        std::vector<int> row;
        for (int x = 0; x < height; x++) {
            row.push_back(-1);
        }
        print_array.push_back(row);
    }

    // Pre-load vector
    for (auto const &node : current_level_) {
        current_level.push_back(node.second.get());
    }

    int old_level = level_;

    LOGD_(logwrap::FileLogger) << header;
    while (!current_level.empty()) {
        // Pull node
        AbstractNode* current_node = current_level.front();
        current_level.pop_front();

        if (old_level != current_node->getLevel()) {
            // print
            boardPrint(print_array);
            // reset
            for (int y = 0; y < width; y++) {for (int x = 0; x < height; x++) {print_array[y][x] = -1;}}
        }

        // Get node info
        int node_level = current_node->getLevel();
        int node_id = current_node->getId();
        float node_value = current_node->getValueH();
        bool is_goal = current_node->representsGoal();

        std::vector<AbstractNode*> children = current_node->getChildren();
        std::map<int, AbstractNode*> neighbours = current_node->getNeighbours();
        std::vector<enginetype::GridCell> represented_cells = current_node->getRepresentedCells();

        // Current node
        msg = "Level: " + std::to_string(node_level) + ", Node: " + std::to_string(node_id); 
        msg += ", value: " + std::to_string(node_value) + ", goal: " + std::to_string(is_goal);
        LOGD_(logwrap::FileLogger) << msg;

        // Neighbours
        msg = "\tNeighbours: ";
        for (auto const &neighbour : neighbours) {
            msg += std::to_string(neighbour.first) + ", ";
        }
        LOGD_(logwrap::FileLogger) << msg;

        // Node Reperesentation
        msg = "\tRepresenting: ";
        for (auto const &rep : represented_cells) {
            msg += "(" + std::to_string(rep.x) + "," + std::to_string(rep.y) + "), ";
            print_array[rep.y][rep.x] = node_id;
        }
        LOGD_(logwrap::FileLogger) << msg;

        // Children
        msg = "\tChildren: ";
        for (auto const &child : children) {
            msg += std::to_string(child->getId()) + ", ";
        }
        LOGD_(logwrap::FileLogger) << msg;

        // Add children and continue
        current_level.insert(current_level.end(), children.begin(), children.end());

        old_level = current_node->getLevel();

    }
    LOGD_(logwrap::FileLogger) << header;
}


void AbstractGraph::abstract() {
    LOGI_(logwrap::FileLogger) << "Abstracting graph.";

    Timer timer;
    timer.start();
    while(current_level_.size() > 1) {
        next_level_.clear();
        nodes_to_delete.clear();
        level_ += 1;

        findCliquesThree();
        findCliquesTwo();
        findCliquesOne();
        joinNeighbours();
        setCurrentFromNext();

        // logGraph();
        // Sanity check
        // PLOGE_IF_(logwrap::FileLogger, !current_level_.empty()) << "Not all nodes could be abstracted.";
    }
    timer.stop();

    LOGI_(logwrap::FileLogger) << "Abstraction complete.";
    LOGI_(logwrap::FileLogger) << "Number of levels: " << level_;
    LOGI_(logwrap::FileLogger) << "Total nodes: " << id_;
    LOGI_(logwrap::FileLogger) << "Abstract Time: " << timer.getDuration();

    logGraph();

    // timer.start();
    // setGoal(3, 1);
    // timer.stop();
    // printGraph();
    // std::cout << "Set Distance Time: " << timer.getDuration() << std::endl;
}


int AbstractGraph::getLevel() {
    return level_;
}