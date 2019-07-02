
#include "bfs.h"


void bfs(std::vector<Action> &solution) {
    // Starting state will be the reference point which all 
    // simulations will be based from
    typedef std::unique_ptr<TreeNodeBFS> Pointer;
    GameState startingState;
    startingState.setFromSimulator();

    // BFS data structures
    std::vector<Pointer> open;
    std::vector<Pointer> closed;
    open.push_back(std::make_unique<TreeNodeBFS>(nullptr));
    solution.clear();

    // Duplication detection
    StateHashTable* hashTable = new StateHashTable();
    hashTable->add(); 

    // Stats
    int count_expanded = 0;
    Timer timer;
    timer.start();

    while (!open.empty()) {
        // Get next node to evaluate
        Pointer node = std::move(open.front());
        open.erase(open.begin());

        count_expanded += 1;

        if (options.debug && count_expanded % 10 == 0) {
            std::cout << "Count: " << count_expanded
            << " Open: " << open.size()
            << " Closed: " << closed.size()
            << std::endl;
        }

        // Set simulator to state off open
        startingState.restoreSimulator();
        node.get()->setSimulatorToCurrent();

        // Game over due to dying?
        if (engineGameFailed()) {
            std::cout << "Simulation: Loss of life." << std::endl;
            continue;
        }

        // Game won
        if (engineGameSolved()) {
            TreeNodeBFS* tempNode = node.get();
            std::cout << "Simulation: Game ended" << std::endl;
            while(tempNode != nullptr && tempNode->getParent() != nullptr) {
                for (int i = 0; i < 7; i++) {
                    solution.insert(solution.begin(), tempNode->getActionTaken());
                }
                solution.insert(solution.begin(), tempNode->getActionTaken());
                tempNode = tempNode->getParent();
            }
            break;
        }

        // Each action gets applied, then simulator gets rolled back to stateBeforeActions
        GameState stateBeforeActions;
        stateBeforeActions.setFromSimulator();

        for (const Action action : ALL_ACTIONS) {
            // Roll back before action taken
            stateBeforeActions.restoreSimulator();

            // Don't add child nodes if player is blocked from moving in that direction
            if (isWall(action)) {continue;}

            // Create child with new action taken
            // This rolls the simulator ahead to the child state
            std::unique_ptr<TreeNodeBFS> childNode = std::make_unique<TreeNodeBFS>(node.get(), node.get()->actions_from_start);
            childNode.get()->setActionTaken(action);

            // Throw away if we already explored this node
            if (!hashTable->in()) {
                hashTable->add();
                open.push_back(std::move(childNode));
            }
        }
        closed.push_back(std::move(node));
    }

    timer.stop();
    int ms = timer.getDuration();

    // int ms = stopClock();  
    std::cout << "BFS ellapsed time: " << ms << "ms" << std::endl;
    std::cout << "BFS total nodes expanded: " << count_expanded << std::endl;
    std::cout << "Size of Open: " << open.size() << std::endl;
    std::cout << "Size of Closed: " << closed.size() << std::endl;
    std::cout << "memory: " << sizeof(board_short) << std::endl;

    // Put simulator back to original state
    startingState.restoreSimulator();

    // Print solution
    std::cout << "solution length: " << solution.size() << std::endl;
    for (int i = 0; i < (int)solution.size(); i++) {
        if (i % 8 != 0) {std::cout << "(";}
        std::cout << actionToString(solution[i]);
        if (i % 8 != 0) {std::cout << ")";} 
        std::cout << " ";
    }
    std::cout << std::endl;
}