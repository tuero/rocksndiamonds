

#include "engine_speed.h"



void engineSpeedNoOptimizations() {
    StartGameActions(network.enabled, setup.autorecord, level.random_seed);
    is_simulating = FALSE;

    Timer timer;
    int num_steps;

    timer.start();
    for (int i = 0; i < MAX_SIMULATIONS; i++) {
        setEngineRandomPlayerAction();
        engineSimulate();
    }

    timer.stop();
    int ms = timer.getDuration();
    int avg_res = ms/MAX_SIMULATIONS;
    int avg_single = avg_res/_ENGINE_RESOLUTION;

    std::cout << "Test: Simulator speed without optimizations" << std::endl;
    std::cout << "Number of simulations: " << MAX_SIMULATIONS << std::endl;
    std::cout << "Total time: " << ms << "us" << std::endl;
    std::cout << "Average time (resolution=" << _ENGINE_RESOLUTION << "): " << avg_res << "us" << std::endl;
    std::cout << "Average time (resolution=1): " << avg_single << "us" << std::endl << std::endl;
}


void engineSpeedWithOptimizations() {
    StartGameActions(network.enabled, setup.autorecord, level.random_seed);
    is_simulating = TRUE;

    Timer timer;
    int num_steps;

    timer.start();
    for (int i = 0; i < MAX_SIMULATIONS; i++) {
        setEngineRandomPlayerAction();
        engineSimulate();
    }

    timer.stop();
    int ms = timer.getDuration();
    int avg_res = ms/MAX_SIMULATIONS;
    int avg_single = avg_res/_ENGINE_RESOLUTION;

    std::cout << "Test: Simulator speed with optimizations" << std::endl;
    std::cout << "Number of simulations: " << MAX_SIMULATIONS << std::endl;
    std::cout << "Total time: " << ms << "us" << std::endl;
    std::cout << "Average time (resolution=" << _ENGINE_RESOLUTION << "): " << avg_res << "us" << std::endl;
    std::cout << "Average time (resolution=1): " << avg_single << "us" << std::endl << std::endl;
}


void bfsSpeed() {
    StartGameActions(network.enabled, setup.autorecord, level.random_seed);
    is_simulating = TRUE;

    int ms;
    int nodes_expanded;
    std::vector<Action> solution;
    bfs(solution, ms, nodes_expanded);

    int avg_res = ms/nodes_expanded;
    int avg_single = avg_res/_ENGINE_RESOLUTION;

    std::cout << "Test: BFS speed" << std::endl;
    std::cout << "Number of nodes expanded: " << nodes_expanded << std::endl;
    std::cout << "Total time: " << ms << "us" << std::endl;
    std::cout << "Average time (resolution=" << _ENGINE_RESOLUTION << "): " << avg_res << "us" << std::endl;
    std::cout << "Average time (resolution=1): " << avg_single << "us" << std::endl << std::endl;
}


void mctsSpeed() {
    StartGameActions(network.enabled, setup.autorecord, level.random_seed);
    is_simulating = TRUE;
    MCTS mcts;
    
    int ms;
    int nodes_expanded;
    int iterations;
    std::vector<Action> solution;
    mcts.run(solution, ms, iterations, nodes_expanded);
    int avg_res = ms/nodes_expanded;
    int avg_single = avg_res/_ENGINE_RESOLUTION;

    std::cout << "Test: MCTS speed" << std::endl;
    std::cout << "Number of iterations: " << iterations << std::endl;
    std::cout << "Number of nodes expanded: " << nodes_expanded << std::endl;
    std::cout << "Total time: " << ms << "us" << std::endl;
    std::cout << "Average time (resolution=" << _ENGINE_RESOLUTION << "): " << avg_res << "us" << std::endl;
    std::cout << "Average time (resolution=1): " << avg_single << "us" << std::endl << std::endl;
}