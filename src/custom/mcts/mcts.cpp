
#include "mcts.h"


MCTS::MCTS(){}


TreeNode* selectBestChild(TreeNode* current) {

}



Action MCTS::run() {
    // Timer gets initialized
    timer.init();

    // Save the current game state from the simulator
    GameState startingState;
    startingState.setFromSimulator();

    // MCTS starting configuration
    current_iterations = 0;
    TreeNode root(nullptr);
    root.setActions();

    // Random Number Generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, NUM_ACTIONS);

    // Always loop until breaking conditions
    while (true) {
        // Step 1: Selection
        // Starting at root, continuously select best child using policy until leaf node
        // is met. A leaf is any node which no simulation has been initiated
        TreeNode* current = &root;
        while (current->allExpanded()) {
            // select best child
            current = selectBestChild(current);
            // update simulator
            setEnginePlayerAction(current->getActionTaken());
            for (int i = 0; i < _ENGINE_RESOLUTION; i++) {
                // Shouldn't need to handle EOL or EOG, as we are using same seed for random events
                engineSimulateSingle();
            }
        }


        // Step 2: Expansion
        // We exand the node if its not terminal and not already fully expanded
        // Need to consider if state is winning state
        if (!current->isTerminal() && !current->allExpanded()) {
            current = current->expand();
        }


        // Step 3: Simulation
        // The expansion step sets the simulator to the expanded node's state
        if (!current->isTerminal()) {
            for (int t = 0; t < max_iterations; t++) {
                // Terminal condition in simulator
                if (engineGameFailed() || engineGameSolved()) {
                    break;
                }

                // Apply random action
                // This is biased but will fix later

            }
        }

        // Step 4: Backpropagation   
    }


    

    // Put simulator back to original state
    startingState.restoreSimulator();
}