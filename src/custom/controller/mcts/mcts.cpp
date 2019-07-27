
#include "mcts.h"


MCTS::MCTS(){
    // Load parameters from file
    std::string line;
    std::string parameter_name;
    float parameter_value;

    msg = "Cannot open mcts configuration file. Falling back to default parameters";

    try{
        std::ifstream configFile (config_dir + mcts_config);
        std::vector<Action> path;
        while (std::getline(configFile,line)){
            std::istringstream iss(line);
            if (!(iss >> parameter_name >> parameter_value)) {
                PLOGE_(logwrap::FileLogger) << "Bad line in config file, skipping.";
                continue;
            }

            // Process parameter
            if (parameter_name == "max_time") {
                max_time = parameter_value;
            }
            else if (parameter_name == "max_iterations_depth") {
                max_iterations_depth = parameter_value;
            }
            else if (parameter_name == "num_simulations") {
                num_simulations = parameter_value;
            }
            else if (parameter_name == "w_distance") {
                w_distance = parameter_value;
            }
            else if (parameter_name == "w_goals_count") {
                w_goals_count = parameter_value;
            }
            else if (parameter_name == "w_died_count") {
                w_died_count = parameter_value;
            }
            else if (parameter_name == "w_safe_count") {
                w_safe_count = parameter_value;
            }
            else if (parameter_name == "w_visit_count") {
                w_visit_count = parameter_value;
            }
        }

        configFile.close();
    }
    catch (const std::ifstream::failure& e) {
        PLOGE_(logwrap::FileLogger) << "Cannot open file.";
    }

    // Log parameters being used
    PLOGD_(logwrap::FileLogger) << "MCTS parametrs...";
    PLOGD_(logwrap::FileLogger) << "max_time: " << max_time;
    PLOGD_(logwrap::FileLogger) << "max_iterations_depth: " << max_iterations_depth;
    PLOGD_(logwrap::FileLogger) << "num_simulations: " << num_simulations;
    PLOGD_(logwrap::FileLogger) << "w_distance: " << w_distance;
    PLOGD_(logwrap::FileLogger) << "w_goals_count: " << w_goals_count;
    PLOGD_(logwrap::FileLogger) << "w_died_count: " << w_died_count;
    PLOGD_(logwrap::FileLogger) << "w_safe_count: " << w_safe_count;
    PLOGD_(logwrap::FileLogger) << "w_visit_count: " << w_visit_count;
}


TreeNode* MCTS::selectBestChild(TreeNode* current) {
    float best_score = -10;
    float best_depth = 1000;
    int best_distance = 1000;
    TreeNode* current_child = nullptr;
    TreeNode* best_child = nullptr;

    GameState reference;
    reference.setFromSimulator();

    for (unsigned int i = 0; i < current->getChildCount(); i++) {
        current_child = current->getChild(i);
        float node_value = nodeValue(current_child);
        int node_distance = current_child->getDistance();

        // Better child node found
        // Determine if child is deadly
        reference.restoreSimulator();
        for (unsigned int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
            enginehelper::setEnginePlayerAction(current_child->getActionTaken());
            enginehelper::engineSimulateSingle();
        }

        if (enginehelper::engineGameFailed()) {
            PLOGE_IF_(logwrap::FileLogger, !current_child->isDeadly()) << "This shouldn't happen.";
            // continue; 
        }


        if (current_child->isDeadly()) {
            std::string msg = "Child " + actionToString(current_child->getActionTaken()) + " is terminal.";
            PLOGD_(logwrap::FileLogger) << msg;
            continue;
        }

        if (node_value > best_score || (node_value == best_score && node_distance < best_distance)) {
            best_score = node_value;
            best_child = current_child;
            best_distance = node_distance;
        }
    }

    if (best_child == nullptr) {
        PLOGE_(logwrap::FileLogger) << " No non-terminal child found.";
    }

    return best_child;
}

TreeNode* MCTS::get_best_uct_child(TreeNode* node) {
    float uct_k = sqrt(2);
    float epsilon = 0.00001;

    float best_utc_score = -std::numeric_limits<float>::max();
    TreeNode* best_node = NULL;

    // iterate all immediate children and find best UTC score
    unsigned int num_children = node->getChildCount();
    for(unsigned int i = 0; i < num_children; i++) {
        TreeNode* child = node->getChild(i);
        float node_value = nodeValue(child);
        float uct_exploitation = node_value / (child->getVisitCount() + epsilon);
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

    for (unsigned int i = 0; i < current->getChildCount(); i++) {
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
    
    float visit_count = (float)current->getVisitCount();
    int depth = current->getDepth();
    float distance = (float)current->getDistance();
    float goals_count = current->getGoalCount();
    float died_count = current->getDiedCount();
    float count_safe = current->getIsSafe();

    value += w_distance * (enginehelper::max_distance - distance);
    value += w_visit_count * visit_count;
    value += w_goals_count * (goals_count / visit_count);
    value -= w_died_count * (died_count / visit_count);
    value += w_safe_count * (count_safe/ visit_count);

    return value;
}



std::string MCTS::childValues(TreeNode* current) {
    std::string output = "";
    TreeNode* current_child = nullptr;

    for (unsigned int i = 0; i < current->getChildCount(); i++) {
        current_child = current->getChild(i);
        float visit_count = (float)current_child->getVisitCount();
        float distance = (float)current_child->getDistance();
        float goals_count = current_child->getGoalCount();
        float died_count = current_child->getDiedCount();
        float count_safe = current_child->getIsSafe();

        output += "\taction: ";
        output += actionToString(current_child->getActionTaken());
        output += ", value: ";
        output += std::to_string(nodeValue(current_child));
        output += ", visit count: ";
        output += std::to_string(visit_count);
        output += ", distance: ";
        output += std::to_string(w_distance * (enginehelper::max_distance - distance));
        output += ", death: ";
        output += std::to_string(-w_died_count * (died_count / visit_count));
        output += ", safe: ";
        output += std::to_string(w_safe_count * (count_safe/ visit_count));
        output += "\n";
    }

    return output;
}


void MCTS::reset(std::vector<Action> &next_action) {
    root = std::make_unique<TreeNode>(nullptr);
    calls_since_rest = 0;
    
    // Reset statistics
    count_simulated_nodes = 0;
    count_expanded_nodes = 0;
    max_depth = 0;

    // Save current state
    GameState reference_state;
    reference_state.setFromSimulator();

    // step forward
    enginehelper::setSimulatorFlag(true);
    std::string msg = "Setting root to next action in queue: ";
    msg += actionToString(next_action[0]);
    for (unsigned int i = 0; i < next_action.size(); i++) {
        enginehelper::setEnginePlayerAction(next_action[i]);
        enginehelper::engineSimulateSingle();
    }

    PLOGD_(logwrap::ConsolLogger) << msg;
    PLOGD_(logwrap::FileLogger) << msg;
    logwrap::logPlayerDetails();
    logwrap::logBoardState();
    logwrap::logMovPosState();
    // save root state to where we will be when done executing current queued action
    rootSavedState.setFromSimulator();
    root.get()->setActions();
    // TreeNode root(nullptr);
    // if (root == nullptr) {
    //     root = std::make_unique<TreeNode>(nullptr);
    // }
    // else {
    //     root = root->getChildByAction(next_action[0]);
    //     root.get()->setParent(nullptr);
    // }

    // reset engine back to reference state
    reference_state.restoreSimulator();
    // RNG::setSeedEngineHash();
    enginehelper::setSimulatorFlag(false);
}


void MCTS::handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) {
    currentSolution = forwardSolution;
    std::string msg = "Resetting MCTS tree.";
    PLOGD_(logwrap::FileLogger) << msg;
    PLOGD_(logwrap::ConsolLogger) << msg;
    reset(currentSolution);
}


void MCTS::run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) 
{
    // Silent compiler warning
    (void)currentSolution;

    // Set simulator flag, allows for optimized simulations
    enginehelper::setSimulatorFlag(true);

    // Timer gets initialized
    timer.init();
    float timeTotal = 0;
    float counter = 0;
    float sim_denom = 1.0 / num_simulations;
    calls_since_rest += 1;

    // Save the current game state from the simulator
    GameState startingState;
    startingState.setFromSimulator();

    // Set current game state to root state
    rootSavedState.restoreSimulator();
    // RNG::setSeedEngineHash();

    // MCTS starting configurations
    TreeNode* best_child = nullptr;

    // Log state before MCTS to ensure we are simulating after queued actions are taken.
    // This allows for a next step partial solution to be given immediately after current
    // actions are finished.
    PLOGD_(logwrap::FileLogger) << "State before MCTS";
    logwrap::logPlayerDetails();
    logwrap::logBoardState();
    logwrap::logMovPosState();

    forwardSolution.clear();
    timer.start();

    // Always loop until breaking conditions
    while (true) {
        // Step 1: Selection
        // Starting at root, continuously select best child using policy until leaf node
        // is met. A leaf is any node which no simulation has been initiated
        TreeNode* current = root.get();
        unsigned int current_depth = 0;

        // Put simulator back to root state
        // !TODO -> Maybe encorporate set seed into restore simulator?
        rootSavedState.restoreSimulator();
        // RNG::setSeedEngineHash();

        msg = "Time remaining: " + std::to_string(timer.getTimeLeft(max_time));
        PLOGD_(logwrap::FileLogger) << msg;
        msg = "Current number of expanded nodes: " + std::to_string(count_expanded_nodes)
                + ", simulated nodes: " + std::to_string(count_simulated_nodes);
        PLOGD_(logwrap::FileLogger) << msg;

        // Select child based on policy and forward engine simulator
        while (!current->getTerminalStatusFromEngine() && current->allExpanded()) {
            current = get_best_uct_child(current);
            enginehelper::setEnginePlayerAction(current->getActionTaken());
            for (unsigned int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
                // Shouldn't need to handle EOL or EOG, as we are using same seed for random events
                enginehelper::engineSimulateSingle();
            }
            current_depth += 1;
        }

        max_depth = (current_depth > max_depth) ? current_depth : max_depth;

        // Step 2: Expansion
        // We exand the node if its not terminal and not already fully expanded
        // Need to consider if state is winning state
        if (!current->getTerminalStatusFromEngine() && !current->allExpanded()) {
            current = current->expand();
            count_expanded_nodes += 1;
        }

        float current_distance = current->getDistance();
        int sim_iterations = 2 * current_distance;
        float times_goal_found = 0;
        float times_died = 0;

        // Before we simulate, we need to save a reference state to get back to
        GameState reference_state;
        reference_state.setFromSimulator();

        // Step 3: Simulation
        // The expansion step sets the simulator to the expanded node's state
        for (unsigned int i = 0; i < num_simulations; i++) {
            reference_state.restoreSimulator();
            // RNG::setSeedEngineHash();
            if (!current->isTerminal()) {
                for (unsigned int t = 0; t < max_iterations_depth; t++) {
                    // Terminal condition in simulator
                    if (enginehelper::engineGameFailed() || enginehelper::engineGameSolved()) {
                        break;
                    }

                    // Apply random action
                    // This is biased but will fix later
                    enginehelper::setEngineRandomPlayerAction();
                    enginehelper::engineSimulate();
                    count_simulated_nodes += 1;
                }
            }
            times_goal_found += enginehelper::engineGameSolved() ? sim_denom : 0;
            times_died += enginehelper::engineGameFailed() ? sim_denom : 0;
        }
        // Check if safe
        reference_state.restoreSimulator();
        // RNG::setSeedEngineHash();
        if (!current->isTerminal() && current->getActionTaken() != Action::noop) {
            for (unsigned int t = 0; t < max_iterations_depth; t++) {
                // Terminal condition in simulator
                if (enginehelper::engineGameFailed()) {
                    break;
                }
                enginehelper::setEnginePlayerAction(Action::noop);
                enginehelper::engineSimulate();
                count_simulated_nodes += 1;
            }
        }

        float safe = (enginehelper::engineGameFailed() ? 0 : 1);

        // float value = nodeValue(current);
        bool goal_found = enginehelper::engineGameSolved();
        bool died = enginehelper::engineGameFailed();

        // Step 4: Backpropagation   
        while (current != nullptr) {
            // current->updateStats(goal_found, died);
            current->updateStats(times_goal_found, times_died, safe);
            current = current->getParent();
        }

        // Exit conditions:
        // Reached max iterations or max time
        counter += 1;
        float avgTime = timeTotal / counter;
        if (timer.getTimeLeft(max_time) < avgTime) {break;}
    }

    timer.stop();
    int ms = timer.getDuration();

    // Update statistics
    statistics[enginetype::RUN_TIME] = ms;
    statistics[enginetype::COUNT_EXPANDED_NODES] = count_expanded_nodes;
    statistics[enginetype::COUNT_SIMULATED_NODES] = count_simulated_nodes;
    statistics[enginetype::MAX_DEPTH] = max_depth;

    // Get best child and its associated action
    rootSavedState.restoreSimulator();
    // RNG::setSeedEngineHash();
    best_child = selectBestChild(root.get());
    Action best_action = (best_child == nullptr ? Action::noop : best_child->getActionTaken());

    // Logg root child nodes along with their current valuation
    msg = "Child node values:\n" + childValues(root.get());
    PLOGD_(logwrap::FileLogger) << msg;
    PLOGD_IF_(logwrap::ConsolLogger, calls_since_rest == 8) << msg;

    // Put simulator back to original state
    startingState.restoreSimulator();
    // RNG::setSeedEngineHash();
    enginehelper::setSimulatorFlag(false);

    // return best action
    forwardSolution.clear();
    for (unsigned int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        forwardSolution.push_back(best_action);
    }
}