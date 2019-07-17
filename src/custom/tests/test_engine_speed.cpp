

#include "test_engine_speed.h"


namespace testenginespeed {

    /*
     * Usage: ./rocksndiamonds -solver test_engine -loadlevel 10
     */
    void testEngineSpeedNoOptimizations() {
        PLOGD_(logwrap::FileLogger) << "Running test: Engine speed without optimizations.";
        PLOGD_(logwrap::FileLogger) << "Level: " << level.file_info.nr;

        StartGameActions(network.enabled, setup.autorecord, level.random_seed);
        enginehelper::setSimulatorFlag(FALSE);

        Timer timer;

        // Save starting state
        GameState start_state;
        start_state.setFromSimulator();


        timer.start();
        for (unsigned int i = 0; i < MAX_SIMULATIONS; i++) {
            // If game ending condition, reset to initial
            if (enginehelper::engineGameFailed() || enginehelper::engineGameSolved()) {
                start_state.restoreSimulator();
            }
            enginehelper::setEngineRandomPlayerAction();
            enginehelper::engineSimulate();
        }

        timer.stop();

        PLOGD_(logwrap::FileLogger) << "Simulation complete. Statistics:";

        int ms = timer.getDuration();
        int avg_res = ms/MAX_SIMULATIONS;
        int avg_single = avg_res/enginetype::ENGINE_RESOLUTION;

        PLOGD_(logwrap::FileLogger) << "Test: Simulator speed without optimizations";
        PLOGD_(logwrap::FileLogger) << "Number of simulations: " << MAX_SIMULATIONS;
        PLOGD_(logwrap::FileLogger) << "Total time: " << ms << "us";
        PLOGD_(logwrap::FileLogger) << "Average time (resolution=" << enginetype::ENGINE_RESOLUTION << "): " << avg_res << "us";
        PLOGD_(logwrap::FileLogger) << "Average time (resolution=1): " << avg_single << "us";

        PLOGD_(logwrap::FileLogger) << "End of test...";
        PLOGD_(logwrap::FileLogger) << "";
    }


    void testEngineSpeedWithOptimizations() {
        PLOGD_(logwrap::FileLogger) << "Running test: Engine speed with optimizations.";
        PLOGD_(logwrap::FileLogger) << "Level: " << level.file_info.nr;

        StartGameActions(network.enabled, setup.autorecord, level.random_seed);
        enginehelper::setSimulatorFlag(TRUE);

        Timer timer;
        
        // Save starting state
        GameState start_state;
        start_state.setFromSimulator();

        timer.start();
        for (unsigned int i = 0; i < MAX_SIMULATIONS; i++) {
            // If game ending condition, reset to initial
            if (enginehelper::engineGameFailed() || enginehelper::engineGameSolved()) {
                start_state.restoreSimulator();
            }
            enginehelper::setEngineRandomPlayerAction();
            enginehelper::engineSimulate();
        }

        timer.stop();

        PLOGD_(logwrap::FileLogger) << "Simulation complete. Statistics:";


        int ms = timer.getDuration();
        int avg_res = ms/MAX_SIMULATIONS;
        int avg_single = avg_res/enginetype::ENGINE_RESOLUTION;

        PLOGD_(logwrap::FileLogger) << "Test: Simulator speed with optimizations";
        PLOGD_(logwrap::FileLogger) << "Number of simulations: " << MAX_SIMULATIONS;
        PLOGD_(logwrap::FileLogger) << "Total time: " << ms << "us";
        PLOGD_(logwrap::FileLogger) << "Average time (resolution=" << enginetype::ENGINE_RESOLUTION << "): " << avg_res << "us";
        PLOGD_(logwrap::FileLogger) << "Average time (resolution=1): " << avg_single << "us";

        PLOGD_(logwrap::FileLogger) << "End of test...";
        PLOGD_(logwrap::FileLogger) << "";
    }


    void testBfsSpeed() {
        PLOGD_(logwrap::FileLogger) << "Running test: BFS.";
        PLOGD_(logwrap::FileLogger) << "Level: " << level.file_info.nr;

        Controller controller(enginetype::BFS);
        StartGameActions(network.enabled, setup.autorecord, level.random_seed);

        // Run BFS
        logwrap::setLogLevel(plog::none);
        controller.getAction();
        logwrap::setLogLevel(plog::debug);

        int ms = controller.getRunTime();
        int nodes_expanded = controller.getCountExpandedNodes();
        int avg_res = ms/nodes_expanded;
        int avg_single = avg_res/enginetype::ENGINE_RESOLUTION;

        PLOGD_(logwrap::FileLogger) << "Test: BFS speed";
        PLOGD_(logwrap::FileLogger) << "Number of nodes expanded: " << nodes_expanded;
        PLOGD_(logwrap::FileLogger) << "Total time: " << ms << "us";
        PLOGD_(logwrap::FileLogger) << "Average time (resolution=" << enginetype::ENGINE_RESOLUTION << "): " << avg_res << "us";
        PLOGD_(logwrap::FileLogger) << "Average time (resolution=1): " << avg_single << "us";

        PLOGD_(logwrap::FileLogger) << "End of test...";
        PLOGD_(logwrap::FileLogger) << "";
    }


    void testMctsSpeed() {
        PLOGD_(logwrap::FileLogger) << "Running test: MCTS.";
        PLOGD_(logwrap::FileLogger) << "Level: " << level.file_info.nr;

        Controller controller(enginetype::MCTS);
        StartGameActions(network.enabled, setup.autorecord, level.random_seed);

        // Run MCTS and temporary turn off internal debugging (we are only interested
        // in the statistics for this)
        logwrap::setLogLevel(plog::none);
        for (unsigned int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
            controller.getAction();
        }
        logwrap::setLogLevel(plog::debug);

        unsigned int ms = controller.getRunTime();
        unsigned int nodes_expanded = controller.getCountExpandedNodes();
        unsigned int nodes_simulated = controller.getCountSimulatedNodes();
        unsigned int max_depth = controller.getMaxDepth();

        unsigned int avg_res = ms/nodes_expanded;
        unsigned int avg_single = avg_res/enginetype::ENGINE_RESOLUTION;

        PLOGD_(logwrap::FileLogger) << "Test: MCTS speed";
        PLOGD_(logwrap::FileLogger) << "Number of nodes expanded: " << nodes_expanded;
        PLOGD_(logwrap::FileLogger) << "Number of nodes simulated: " << nodes_simulated;
        PLOGD_(logwrap::FileLogger) << "Max depth explored: " << max_depth;
        PLOGD_(logwrap::FileLogger) << "Total time: " << ms << "us";
        PLOGD_(logwrap::FileLogger) << "Average time (resolution=" << enginetype::ENGINE_RESOLUTION << "): " << avg_res << "us";
        PLOGD_(logwrap::FileLogger) << "Average time (resolution=1): " << avg_single << "us";

        PLOGD_(logwrap::FileLogger) << "End of test...";
        PLOGD_(logwrap::FileLogger) << "";
    }

}