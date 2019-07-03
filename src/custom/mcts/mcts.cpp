
#include "mcts.h"


MCTS::MCTS(){}


TreeNode* MCTS::selectBestChild(TreeNode* current) {
    float best_score = -10;
    float best_depth = 1000;
    TreeNode* current_child = nullptr;
    TreeNode* best_child = nullptr;

    for (int i = 0; i < current->getChildCount(); i++) {
        current_child = current->getChild(i);

        // Better child node found
        if (current_child->getValue() > best_score || 
            (current_child->getValue() == best_score && current_child->getDepthFound() < best_depth)) {
            best_score = current_child->getValue();
            best_child = current_child;
            best_depth = current_child->getDepthFound();
        }
    }

    return best_child;
}

TreeNode* get_best_uct_child(TreeNode* node) {
    float uct_k = sqrt(2);
    float epsilon = 0.00001;

    float best_utc_score = -std::numeric_limits<float>::max();
    TreeNode* best_node = NULL;

    // iterate all immediate children and find best UTC score
    int num_children = node->getChildCount();
    for(int i = 0; i < num_children; i++) {
        TreeNode* child = node->getChild(i);
        float uct_exploitation = (float)child->getValue() / (child->getVisitCount() + epsilon);
        float uct_exploration = sqrt( log((float)node->getVisitCount() + 1) / (child->getVisitCount() + epsilon) );
        float uct_score = uct_exploitation + uct_k * uct_exploration;

        if(uct_score > best_utc_score) {
            best_utc_score = uct_score;
            best_node = child;
        }
    }

    return best_node;
}


TreeNode* MCTS::selectMostVisitedChild(TreeNode* current) {
    int best_count = -1;
    float best_distance = 100;
    TreeNode* current_child = nullptr;
    TreeNode* most_visited_child = nullptr;

    for (int i = 0; i < current->getChildCount(); i++) {
        current_child = current->getChild(i);

        // Better child node found
        if (current_child->getVisitCount() > best_count || 
            (current_child->getVisitCount() == best_count && current_child->getDistance() < best_distance)) {
            best_count = current_child->getVisitCount();
            best_distance = current_child->getDistance();
            most_visited_child = current_child;
        }
    }

    return most_visited_child;
}


float MCTS::nodeValue(TreeNode* current) {
    float value = 0.0;
    if (engineGameSolved()) {
        value += 10.0;
    }

    value += (1.0/ (getDistanceToGoal()+1));

    return value;
}



std::string childValues(TreeNode* current) {
    std::string output = "";
    TreeNode* current_child = nullptr;

    for (int i = 0; i < current->getChildCount(); i++) {
        current_child = current->getChild(i);

        output += "action: ";
        output += actionToString(current_child->getActionTaken()) + " ";
        output += "value: ";
        output += std::to_string(current_child->getValue()) + " ";
        output += "visit count: ";
        output += std::to_string(current_child->getVisitCount()) + "\n";
    }

    return output;
}




void MCTS::run(std::vector<Action> &solution, int &total_time, int &iterations, int &nodes_expanded) {
    setSimulatorFlag(true);

    // Timer gets initialized
    timer.init();

    // Save the current game state from the simulator
    GameState startingState;
    startingState.setFromSimulator();

    // MCTS starting configuration
    count_iterations = 0;
    count_nodes = 0;
    TreeNode root(nullptr);
    root.setActions();
    TreeNode* best_child = nullptr;

    // Random Number Generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, NUM_ACTIONS);

    timer.start();
    solution.clear();

    // Always loop until breaking conditions
    while (true) {

        // std::cout << "MCTS: iteration " << count_iterations << std::endl;
        // Step 1: Selection
        // Starting at root, continuously select best child using policy until leaf node
        // is met. A leaf is any node which no simulation has been initiated
        TreeNode* current = &root;

        // Put simulator back to root state
        startingState.restoreSimulator();

        while (!current->isTerminal() && current->allExpanded()) {
            // std::cout << "MCTS: Selection -> Expanding node." << std::endl;
            // select best child

            // current = selectBestChild(current);
            current = get_best_uct_child(current);
            // update simulator
            setEnginePlayerAction(current->getActionTaken());
            for (int i = 0; i < _ENGINE_RESOLUTION; i++) {
                // Shouldn't need to handle EOL or EOG, as we are using same seed for random events
                engineSimulateSingle();
            }
        }

        // std::cout << "MCTS: Expansion -> Selecting best node" << std::endl;
        // Step 2: Expansion
        // We exand the node if its not terminal and not already fully expanded
        // Need to consider if state is winning state
        if (!current->isTerminal() && !current->allExpanded()) {
            current = current->expand();
        }

        // std::cout << "MCTS: Simulation -> Playing randomly" << std::endl;
        float current_distance = current->getDistance();
        int sim_iterations = 2 * current_distance;

        // Step 3: Simulation
        // The expansion step sets the simulator to the expanded node's state
        if (!current->isTerminal()) {
            for (unsigned int t = 0; t < max_iterations; t++) {
                // std::cout << "MCTS: Simulation -> Step " << t << std::endl;
                // Terminal condition in simulator
                if (engineGameFailed() || engineGameSolved()) {
                    break;
                }

                // Apply random action
                // This is biased but will fix later
                setEngineRandomPlayerAction();
                engineSimulate();
                count_nodes += 1;
            }
        }

        // debug_print();
        float value = nodeValue(current);
        int depth_found = current->getDepth();
        // std::cout << "MCTS: Simulation -> Current node value " << value << std::endl;

        // Step 4: Backpropagation   
        // std::cout << "MCTS: Backpropagation" << std::endl;
        while (current != nullptr) {
            current->updateStats(value, depth_found);
            current = current->getParent();
        }

        // std::cout << "MCTS: Selecting best action" << std::endl;
        best_child = selectMostVisitedChild(&root);
        // best_child = selectBestChild(&root);
        // std::cout << "MCTS: Best action" << actionToString(best_child->getActionTaken()) << std::endl;

        // Exit conditions:
        // Reached max iterations or max time
        if (timer.checkTime() >= max_time) {break;}

        count_iterations += 1;
    }

    timer.stop();
    int ms = timer.getDuration();

    total_time = ms;
    iterations = count_iterations;
    nodes_expanded = count_nodes;

    Action best_action = (best_child == nullptr ? Action::noop : best_child->getActionTaken());

    if (options.debug || 1==1) {
        std::cout << "MCTS ellapsed time: " << ms << "ms" << std::endl;
        std::cout << "MCTS iterations: " << count_iterations << std::endl;
        std::cout << "MCTS total nodes expanded: " << count_nodes << std::endl;
        std::cout << "Best action: " << actionToString(best_action) << std::endl;
        std::cout << "Child node values: " << std::endl; 
        std::cout << childValues(&root);
        std::cout << std::endl;
    }

    // Put simulator back to original state
    startingState.restoreSimulator();
    setSimulatorFlag(false);

    // return best action
    for (int i = 0; i < _ENGINE_RESOLUTION; i++) {
        solution.push_back(best_action);
    }
}