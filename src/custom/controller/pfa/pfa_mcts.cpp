
#include "pfa_mcts.h"


PFA_MCTS::PFA_MCTS(){
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



PFATreeNode* PFA_MCTS::get_best_uct_child(PFATreeNode* node) {
    float uct_k = sqrt(2);
    float epsilon = 0.00001;

    float best_utc_score = std::numeric_limits<float>::min();
    PFATreeNode* best_node = NULL;

    // iterate all immediate children and find best UTC score
    unsigned int num_children = node->getChildCount();
    for(unsigned int i = 0; i < num_children; i++) {
        PFATreeNode* child = node->getChild(i);
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


PFATreeNode* PFA_MCTS::selectMostVisitedChild(PFATreeNode* current) {
    int best_count = -1;
    float best_distance = 100;
    PFATreeNode* current_child = nullptr;
    PFATreeNode* most_visited_child = nullptr;

    for (unsigned int i = 0; i < current->getChildCount(); i++) {
        current_child = current->getChild(i);
        if (current_child->isDeadly()) {continue;}

        // Better child node found
        if (current_child->getVisitCount() > best_count || 
            (current_child->getVisitCount() == best_count && current_child->getMinDistance() < best_distance)) {
            best_count = current_child->getVisitCount();
            best_distance = current_child->getMinDistance();
            most_visited_child = current_child;
        }
    }

    return most_visited_child;
}


float PFA_MCTS::nodeValue(PFATreeNode* current) {
    float value = 0.0;
    // float visit_count = current->getVisitCount();
    // float distance = current->getMinDistance();
    int l1_distance = current->getL1Distance();
    float survival_frequency = current->getSurvivalFrequency();
    int min_depth = current->getMinDepthToGoal();

    if (min_depth == std::numeric_limits<int>::max()) {min_depth = max_depth;}

    value += max_depth - min_depth;
    value += allowed_cells_.size() - l1_distance;
    value += survival_frequency * 10;

    return value;
}



std::string PFA_MCTS::childValues(PFATreeNode* current) {
    std::string output = "";
    PFATreeNode* current_child = nullptr;

    for (unsigned int i = 0; i < current->getChildCount(); i++) {
        current_child = current->getChild(i);
        int visit_count = current_child->getVisitCount();
        int l1_distance = current->getL1Distance();
        float survival_frequency = current->getSurvivalFrequency();
        int min_depth = current->getMinDepthToGoal();

        if (min_depth == std::numeric_limits<int>::max()) {min_depth = -1;}

        output += "\taction: ";
        output += actionToString(current_child->getActionTaken());
        output += ", value: ";
        output += std::to_string(nodeValue(current_child));
        output += ", visit count: ";
        output += std::to_string(visit_count);
        output += ", l1_distance: ";
        output += std::to_string(l1_distance);
        output += ", survival: ";
        output += std::to_string(survival_frequency);
        output += ", min_depth: ";
        output += std::to_string(min_depth);
        output += "\n";
    }

    return output;
}


void PFA_MCTS::reset(std::vector<Action> &next_action) {    
    // Reset statistics
    calls_since_rest = 0;
    count_simulated_nodes = 0;
    count_expanded_nodes = 0;
    max_depth = 0;

    // Save current state
    GameState reference_state;
    reference_state.setFromSimulator();

    // step forward to where agent will be after executing current queued move
    enginehelper::setSimulatorFlag(true);
    std::string msg = "Setting root to next action in queue: ";
    msg += actionToString(next_action[0]);
    for (unsigned int i = 0; i < next_action.size(); i++) {
        enginehelper::setEnginePlayerAction(next_action[i]);
        enginehelper::engineSimulateSingle();
    }

    logCurrentState(msg, true);

    // save root state to where we will be when done executing current queued action
    root = std::make_unique<PFATreeNode>(nullptr);
    rootSavedState.setFromSimulator();
    root.get()->setActions(allowed_cells_);

    // reset engine back to reference state
    reference_state.restoreSimulator();
    enginehelper::setSimulatorFlag(false);
}


void PFA_MCTS::handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution,
     AbstractNode* current_abstract_node, AbstractNode* goal_abstract_node) 
{
    currentSolution = forwardSolution;
    std::string msg = "Resetting MCTS tree.";
    PLOGD_(logwrap::FileLogger) << msg;
    PLOGD_(logwrap::ConsolLogger) << msg;

    // current_abstract_node_ = current_abstract_node;
    // goal_abstract_node_ = goal_abstract_node;
    allowed_cells_.clear();
    goal_cells_.clear();

    // Set cells which we are restricted to being in
    for (auto const & cell : current_abstract_node->getRepresentedCells()) {
        allowed_cells_.push_back(cell);
    }
    if (current_abstract_node != goal_abstract_node) {
      for (auto const & cell : goal_abstract_node->getRepresentedCells()) {
            allowed_cells_.push_back(cell);
        }  
    }

    // If we are in goal abstract node, then we only care about the goal tile
    if (current_abstract_node == goal_abstract_node) {
        assert(goal_abstract_node->representsGoal());
        for (auto const & grid_cell : goal_abstract_node->getRepresentedCells()) {
            if (enginehelper::getGridDistanceToGoal(grid_cell) == 0) {
                goal_cells_.push_back(grid_cell);
                break;
            }
        }
    }
    // Otherwise, goal nodes are the nodes which boarder our current abstract node
    else {
        std::vector<enginetype::GridCell> current_cells = current_abstract_node->getRepresentedCells();
        std::vector<enginetype::GridCell> goal_cells = goal_abstract_node->getRepresentedCells();
        for (auto const & goal_cell : goal_cells) {
            for (auto const & current_cell : current_cells) {
                if (enginehelper::checkIfNeighbours(goal_cell, current_cell)) {
                    goal_cells_.push_back(goal_cell);
                    break;
                }
            }
        }
    }

    enginehelper::setAbstractNodeDistances(goal_cells_, allowed_cells_);
    reset(currentSolution);
}



void PFA_MCTS::logCurrentStats() {
    PLOGD_(logwrap::FileLogger) << "Time remaining: " << timer.getTimeLeft(max_time);
    PLOGD_(logwrap::FileLogger) << "Current number of expanded nodes: " << count_expanded_nodes 
        << ", simulated nodes: " << count_simulated_nodes;
}


void PFA_MCTS::logCurrentState(std::string msg, bool send_to_consol) {
    PLOGD_(logwrap::FileLogger) << msg;
    PLOGD_IF_(logwrap::ConsolLogger, send_to_consol) << msg;

    logwrap::logPlayerDetails();
    logwrap::logBoardState();
    logwrap::logMovPosState();
}


void PFA_MCTS::run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) 
{
    // Silent compiler warning
    (void)currentSolution;

    // Set simulator flag, allows for optimized simulations
    enginehelper::setSimulatorFlag(true);

    // Timer gets initialized
    timer.init();
    int loop_counter = 0;
    float sim_denom = 1.0 / num_simulations;
    calls_since_rest += 1;

    // Save the current game state from the simulator
    GameState startingState;
    startingState.setFromSimulator();

    // Set current game state to root state
    rootSavedState.restoreSimulator();

    // MCTS starting configurations
    PFATreeNode* best_child = nullptr;

    // Log state before MCTS to ensure we are simulating after queued actions are taken.
    // This allows for a next step partial solution to be given immediately after current
    // actions are finished.
    logCurrentState("State before MCTS.", false);

    forwardSolution.clear();
    timer.start();

    // Always loop until breaking conditions
    while (true) {
        // Step 1: Selection
        // Starting at root, continuously select best child using policy until leaf node
        // is met. A leaf is any node which no simulation has been initiated
        PFATreeNode* current = root.get();
        unsigned int current_depth = 0;

        // Put simulator back to root state
        rootSavedState.restoreSimulator();
        logCurrentStats();

        // Select child based on policy and forward engine simulator
        while (!current->getTerminalStatusFromEngine() && current->allExpanded()) {
            current = get_best_uct_child(current);
            enginehelper::setEnginePlayerAction(current->getActionTaken());
            enginehelper::engineSimulate();
            current_depth += 1;
        }
        max_depth = (current_depth > max_depth) ? current_depth : max_depth;

        // Step 2: Expansion
        // We exand the node if its not terminal and not already fully expanded
        // Need to consider if state is winning state
        if (!current->getTerminalStatusFromEngine() && !current->allExpanded()) {
            current = current->expand(allowed_cells_, goal_cells_);
            count_expanded_nodes += 1;
        }

        float times_goal_found = 0;
        float times_died = 0;

        // Before we simulate, we need to save a reference state to get back to
        GameState reference_state;
        reference_state.setFromSimulator();
        

        // Step 3: Simulation
        // The expansion step sets the simulator to the expanded node's state
        num_simulations = 0;
        // for (unsigned int i = 0; i < num_simulations; i++) {
        //     reference_state.restoreSimulator();

        //     if (!current->isTerminal()) {
        //         for (unsigned int t = 0; t < max_iterations_depth; t++) {
        //             // Terminal condition in simulator
        //             if (enginehelper::engineGameFailed() || enginehelper::engineGameSolved()) {
        //                 break;
        //             }

        //             // Apply random action
        //             // This is biased but will fix later
        //             enginehelper::setEngineRandomPlayerAction();
        //             enginehelper::engineSimulate();
        //             count_simulated_nodes += 1;
        //         }
        //     }

        //     int min_distance = enginehelper::minDistanceToAllowedCells(goal_cells_);
        //     times_goal_found += (min_distance == 0);

        //     times_died += enginehelper::engineGameFailed() ? sim_denom : 0;
        // }

        // Step 4: Backpropagation
        int min_distance_found = current->getMinDepthToGoal();
        float survival_frequency = current->getSurvivalFrequency();
        int depth = current->getDepth();
        while (current != nullptr) {
            // current->updateStats(times_goal_found, times_died, 0, 0, min_distance_found);
            current->updateStats(times_goal_found, survival_frequency, depth, 0, min_distance_found);
            current = current->getParent();
        }

        // Exit conditions:
        // Reached max iterations or max time
        loop_counter += 1;
        float avgTime = timer.getDuration() / loop_counter;
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
    best_child = selectMostVisitedChild(root.get());
    Action best_action = (best_child == nullptr ? Action::noop : best_child->getActionTaken());

    // Log root child nodes along with their current valuation
    msg = "Child node values:\n" + childValues(root.get());
    PLOGD_(logwrap::FileLogger) << msg;
    PLOGD_IF_(logwrap::ConsolLogger, calls_since_rest == 8) << msg;

    // Put simulator back to original state
    startingState.restoreSimulator();
    enginehelper::setSimulatorFlag(false);

    // return best action
    forwardSolution.clear();
    for (unsigned int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
        forwardSolution.push_back(best_action);
    }
}