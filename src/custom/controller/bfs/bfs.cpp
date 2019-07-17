
#include "bfs.h"


BFS::BFS() {}


/*
 * Get the number of nodes expanded during BFS
 */
unsigned int BFS::getCountNodes() const {
    return count_expanded_nodes;
}


void BFS::handleEmpty(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution) {
    // Silent compiler warning
    (void)currentSolution;
    (void)forwardSolution;
}


/*
 * Run BFS and return a solution
 */
void BFS::run(std::vector<Action> &currentSolution, std::vector<Action> &forwardSolution, 
        std::map<enginetype::Statistics, int> &statistics) 
{
    // Silent compiler warning
    (void)forwardSolution;

    // We already have a solution
    if (!currentSolution.empty()) {
        return;
    }

    enginehelper::setSimulatorFlag(true);

    PLOGD_(logwrap::FileLogger) << "Starting BFS.";

    // Starting state will be the reference point which all 
    // simulations will be based from
    typedef std::unique_ptr<TreeNodeBFS> Pointer;
    GameState startingState;
    startingState.setFromSimulator();

    // BFS data structures
    std::vector<Pointer> open;
    std::vector<Pointer> closed;
    open.push_back(std::make_unique<TreeNodeBFS>(nullptr));
    currentSolution.clear();

    // Duplication detection
    StateHashTable* hashTable = new StateHashTable();
    hashTable->add(); 

    // Stats
    count_expanded_nodes = 0;
    Timer timer;
    timer.start();

    while (!open.empty()) {
        // Get next node to evaluate
        Pointer node = std::move(open.front());
        open.erase(open.begin());

        count_expanded_nodes += 1;

        // Set simulator to state off open
        startingState.restoreSimulator();
        node.get()->setSimulatorToCurrent();

        // Game over due to dying?
        if (enginehelper::engineGameFailed()) {
            PLOGD_(logwrap::FileLogger) << "Simulation: Loss of life.";
            continue;
        }

        // Game won
        if (enginehelper::engineGameSolved()) {
            TreeNodeBFS* tempNode = node.get();
            PLOGD_(logwrap::FileLogger) << "Simulation: Game ended.";
            while(tempNode != nullptr && tempNode->getParent() != nullptr) {
                for (int i = 0; i < 7; i++) {
                    currentSolution.insert(currentSolution.begin(), tempNode->getActionTaken());
                }
                currentSolution.insert(currentSolution.begin(), tempNode->getActionTaken());
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
            if (enginehelper::isWall(action)) {continue;}

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

    PLOGD_(logwrap::FileLogger) << "BFS complete.";

    statistics[enginetype::RUN_TIME] = ms;
    statistics[enginetype::COUNT_EXPANDED_NODES] = count_expanded_nodes;

    // if (options.debug) {
    //     std::cout << "BFS ellapsed time: " << ms << "ms" << std::endl;
    //     std::cout << "BFS total nodes expanded: " << count_expanded_nodes << std::endl;
    //     std::cout << "Size of Open: " << open.size() << std::endl;
    //     std::cout << "Size of Closed: " << closed.size() << std::endl;
    //     std::cout << "memory: " << sizeof(enginetype::board_short) << std::endl; 
    // }  

    // Put simulator back to original state
    startingState.restoreSimulator();
    enginehelper::setSimulatorFlag(false);

    // Print solution
    PLOGD_(logwrap::FileLogger) << "Solution length: " << currentSolution.size();
    PLOGD_(logwrap::ConsolLogger) << "Solution length: " << currentSolution.size();
}