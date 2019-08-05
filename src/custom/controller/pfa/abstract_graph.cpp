

#include "abstract_graph.h"


AbstractGraph::AbstractGraph() {
    // Populate base level with node representing each grid
    int width = enginehelper::getLevelWidth();
    int height = enginehelper::getLevelHeight();
    int level = 0;

    Timer timer;
    timer.start();

    // Create a node for each map grid tile
    for (int y = 0; y < width; y++) {
        for (int x = 0; x < height; x++) {
            float value = (float)enginehelper::distances[x][y];
            int id = next_level_.size();

            // Distance of -1 indicates a stationary wall
            if (value == -1) {
                continue;
            }

            // Create and store node
            enginetype::GridCell grid_cell = {x,y};
            Pointer node = std::make_unique<AbstractNode>(id, level, value, grid_cell);
            next_level_[id] = std::move(node);
        }
    }

    // All nodes are created, now we can start joining neighbours
    joinNeighbours();

    // Set current_level_ to next_level_ 
    setCurrentFromNext();

    timer.stop();
    std::cout << "Time: " << timer.getDuration() << std::endl;

    // Test
    std::cout << current_level_.size() << std::endl;
    for (auto const &node : current_level_) {
        int current_id = node.first;
        AbstractNode* current_node = node.second.get();
        std::map<int, AbstractNode*> neighbours = current_node->getNeighbours();
        std::vector<enginetype::GridCell> represented_cells = current_node->getRepresentedCells();

        std::cout << current_id << " Node (" << represented_cells[0].x << "," << represented_cells[0].y;
        std::cout << "), Neighbours: ";
        for (auto const &neighbour : neighbours) {
            AbstractNode* second_node = neighbour.second;
            std::vector<enginetype::GridCell> represented_cells = second_node->getRepresentedCells();
            std::cout << "(" << represented_cells[0].x << "," << represented_cells[0].y << "), ";
        }
        std::cout << "\n";
    }

}


void AbstractGraph::findCliquesThree() {
    for (it = current_level_.begin(); it != current_level_.end(); ) {
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
            if (neighbour_id < current_id) {continue;}

            // If neighbour is not in current_level_, then it has been previously used
            // in another clique and so we skip to another neighbour
            auto search = current_level_.find(neighbour_id);
            if (search == current_level_.end()) {continue;}

            // Loop until we find 3rd node for clique
            for (auto const &second_neighbour : second_neighbours) {
                int second_neighbour_id = second_neighbour.first;

                // Again, if 2nd neighbour id < current_neighbours, then it was already considered
                if (second_neighbour_id < current_id || second_neighbour_id < neighbour_id) {continue;}

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
                Pointer node = std::make_unique<AbstractNode>(id, level, children);
                next_level_[id] = std::move(node);

                // Erase clique from current level
                current_level_.erase(second_neighbour_id);
                current_level_.erase(neighbour_id);
                it = current_level_.erase(it);
                removed = true;
                break;
            }

        }

        // If we removed then we do not increment iterator
        if (!removed) {++it;}
    }
}


void AbstractGraph::findCliquesTwo() {
    for (it = current_level_.begin(); it != current_level_.end(); ) {
        bool removed = false;
        std::map<int, AbstractNode*> current_neighbours = it->second.get()->getNeighbours();

        // Node not in a clique of size 2 if it has fewer than 1 neighbours
        if (current_neighbours.size() < 1) {continue;}

        // Check for clique of size 2
        for (auto const &neighbour : current_neighbours) {
            int neighbour_id = neighbour.first;

            // If neighbour is not in current_level_, then it has been previously used
            // in another clique and so we skip to another neighbour
            auto search = current_level_.find(neighbour_id);
            if (search == current_level_.end()) {continue;}

            // Otherwise, neighbour is free to be used in clique
            std::vector<Pointer> children;
            children.push_back(std::move(it->second));
            children.push_back(std::move(search->second));

            // Make node and store
            int id = next_level_.size();
            Pointer node = std::make_unique<AbstractNode>(id, level, children);
            next_level_[id] = std::move(node);

            // Erase clique from current level
            current_level_.erase(neighbour_id);
            it = current_level_.erase(it);
            removed = true;
            break;
        }

        // If we removed then we do not increment iterator
        if (!removed) {++it;}
    }
}


void AbstractGraph::findCliquesOne() {
    // Remaining nodes were not a part of larger size cliques
    // so we abstract themselves
    for (it = current_level_.begin(); it != current_level_.end(); ++it) {
        int id = next_level_.size();

        // Don't use initialization list as it uses copy-constructor
        // and breaks references
        std::vector<Pointer> children;
        children.push_back(std::move(it->second));

        // Make node and store
        Pointer node = std::make_unique<AbstractNode>(id, level, children);
        next_level_[id] = std::move(node);
    }
}


void AbstractGraph::joinNeighbours() {
    for (it = next_level_.begin(); it != next_level_.end(); ++it) {
            for (auto it_inner = std::next(it); it_inner != next_level_.end(); ++it_inner) {
                AbstractNode* node_left = it->second.get();
                AbstractNode* node_right = it_inner->second.get();

                std::vector<enginetype::GridCell> cells_left = node_left->getRepresentedCells();
                std::vector<enginetype::GridCell> cells_right = node_right->getRepresentedCells();

                if (it->first == 225) {
                    std::cout << cells_left.size() << " " << cells_right.size() << " "; 
                    std::cout << enginehelper::checkIfNeighbours(cells_left, cells_right);
                    std::cout << std::endl;
                }

                if (enginehelper::checkIfNeighbours(cells_left, cells_right)) {
                    node_left->addNeighbour(node_right);
                    node_right->addNeighbour(node_left);
                }
            }
        }
}

void AbstractGraph::setCurrentFromNext() {
    current_level_.clear();
    for (it = next_level_.begin(); it != next_level_.end(); ++it) {
        current_level_[it->first] = std::move(it->second);
    }
    next_level_.clear();
}


void AbstractGraph::setGoal(int goal_x, int goal_y) {
    // Run Dijkstra to set distances
    enginehelper::setBoardDistances(goal_x, goal_y);

    // Update nodes in graph

}


void AbstractGraph::printGraph() {
    std::deque<AbstractNode*> current_level;

    // Pre-load vector
    for (auto const &node : current_level_) {
        current_level.push_back(node.second.get());
    }

    while (!current_level.empty()) {
        // Pull node
        AbstractNode* current_node = current_level.front();
        current_level.pop_front();

        // Get node info
        int node_level = current_node->getLevel();
        int node_id = current_node->getId();
        float node_value = current_node->getValue();
        std::vector<AbstractNode*> children = current_node->getChildren();
        std::map<int, AbstractNode*> neighbours = current_node->getNeighbours();
        std::vector<enginetype::GridCell> represented_cells = current_node->getRepresentedCells();

        // Display node information
        std::cout << "Level: " << node_level << ", Node: " << node_id << ", value: " << node_value << std::endl;
        std::cout << "\t" << "Neighbours: ";
        for (auto const &neighbour : neighbours) {
            std::cout << neighbour.first << ", ";
        }
        std::cout << std::endl;
        std::cout << "\t" << "Representing: ";
        for (auto const &rep : represented_cells) {
            std::cout << "(" << rep.x << "," << rep.y << "), ";
        }
        std::cout << std::endl;

        // Add children and continue
        current_level.insert(current_level.end(), children.begin(), children.end());

    }
}


void AbstractGraph::abstract() {
    Timer timer;
    timer.start();
    while(current_level_.size() > 1) {
        next_level_.clear();
        level += 1;

        findCliquesThree();
        findCliquesTwo();
        findCliquesOne();

        joinNeighbours();
        setCurrentFromNext();

        // Sanity check
        PLOGE_IF_(logwrap::FileLogger, !current_level_.empty()) << "Not all nodes could be abstracted.";
    }

    printGraph();

    timer.stop();
    std::cout << "Time: " << timer.getDuration() << std::endl;
}