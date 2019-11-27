

#include "test_engine_speed.h"
#include "../controller/controller_listing.h"

namespace testenginespeed {

    /*
     * Usage: ./rocksndiamonds -solver test_engine -loadlevel 10
     */
    void testEngineSpeedNoOptimizations() {
        PLOGD_(logger::FileLogger) << "Running test: Engine speed without optimizations.";
        PLOGD_(logger::FileLogger) << "Level: " << level.file_info.nr;

        StartGameActions(network.enabled, setup.autorecord, level.random_seed);
        enginehelper::setSimulatorFlag(FALSE);

        Timer timer;

        // Save starting state
        GameState start_state;
        start_state.setFromEngineState();


        timer.start();
        for (unsigned int i = 0; i < MAX_SIMULATIONS; i++) {
            // If game ending condition, reset to initial
            if (enginehelper::engineGameFailed() || enginehelper::engineGameSolved()) {
                start_state.restoreEngineState();
            }
            enginehelper::setEngineRandomPlayerAction();
            enginehelper::engineSimulate();
        }

        timer.stop();

        PLOGD_(logger::FileLogger) << "Simulation complete. Statistics:";

        int ms = timer.getDuration();
        int avg_res = ms/MAX_SIMULATIONS;
        int avg_single = avg_res/enginetype::ENGINE_RESOLUTION;

        PLOGD_(logger::FileLogger) << "Test: Simulator speed without optimizations";
        PLOGD_(logger::FileLogger) << "Number of simulations: " << MAX_SIMULATIONS;
        PLOGD_(logger::FileLogger) << "Total time: " << ms << "us";
        PLOGD_(logger::FileLogger) << "Average time (resolution=" << enginetype::ENGINE_RESOLUTION << "): " << avg_res << "us";
        PLOGD_(logger::FileLogger) << "Average time (resolution=1): " << avg_single << "us";

        PLOGD_(logger::FileLogger) << "End of test...";
        PLOGD_(logger::FileLogger) << "";
    }


    void testEngineSpeedWithOptimizations() {
        PLOGD_(logger::FileLogger) << "Running test: Engine speed with optimizations.";
        PLOGD_(logger::FileLogger) << "Level: " << level.file_info.nr;

        StartGameActions(network.enabled, setup.autorecord, level.random_seed);
        enginehelper::setSimulatorFlag(TRUE);

        Timer timer;
        
        // Save starting state
        GameState start_state;
        start_state.setFromEngineState();

        timer.start();
        for (unsigned int i = 0; i < MAX_SIMULATIONS; i++) {
            // If game ending condition, reset to initial
            if (enginehelper::engineGameFailed() || enginehelper::engineGameSolved()) {
                start_state.restoreEngineState();
            }
            enginehelper::setEngineRandomPlayerAction();
            enginehelper::engineSimulate();
        }

        timer.stop();

        PLOGD_(logger::FileLogger) << "Simulation complete. Statistics:";


        int ms = timer.getDuration();
        int avg_res = ms/MAX_SIMULATIONS;
        int avg_single = avg_res/enginetype::ENGINE_RESOLUTION;

        PLOGD_(logger::FileLogger) << "Test: Simulator speed with optimizations";
        PLOGD_(logger::FileLogger) << "Number of simulations: " << MAX_SIMULATIONS;
        PLOGD_(logger::FileLogger) << "Total time: " << ms << "us";
        PLOGD_(logger::FileLogger) << "Average time (resolution=" << enginetype::ENGINE_RESOLUTION << "): " << avg_res << "us";
        PLOGD_(logger::FileLogger) << "Average time (resolution=1): " << avg_single << "us";

        PLOGD_(logger::FileLogger) << "End of test...";
        PLOGD_(logger::FileLogger) << "";
    }


    void testMctsSpeed() {
        PLOGD_(logger::FileLogger) << "Running test: MCTS.";
        PLOGD_(logger::FileLogger) << "Level: " << level.file_info.nr;

        Controller controller(CONTROLLER_MCTS);
        StartGameActions(network.enabled, setup.autorecord, level.random_seed);

        // Run MCTS and temporary turn off internal debugging (we are only interested
        // in the statistics for this)
        logger::setLogLevel(logger::LogLevel::none);
        for (unsigned int i = 0; i < enginetype::ENGINE_RESOLUTION; i++) {
            controller.getAction();
        }
        logger::setLogLevel(logger::LogLevel::debug);

        // unsigned int ms = controller.getRunTime();
        // unsigned int nodes_expanded = controller.getCountExpandedNodes();
        // unsigned int nodes_simulated = controller.getCountSimulatedNodes();
        // unsigned int max_depth = controller.getMaxDepth();

        // unsigned int avg_res = ms/nodes_expanded;
        // unsigned int avg_single = avg_res/enginetype::ENGINE_RESOLUTION;

        // PLOGD_(logger::FileLogger) << "Test: MCTS speed";
        // PLOGD_(logger::FileLogger) << "Number of nodes expanded: " << nodes_expanded;
        // PLOGD_(logger::FileLogger) << "Number of nodes simulated: " << nodes_simulated;
        // PLOGD_(logger::FileLogger) << "Max depth explored: " << max_depth;
        // PLOGD_(logger::FileLogger) << "Total time: " << ms << "us";
        // PLOGD_(logger::FileLogger) << "Average time (resolution=" << enginetype::ENGINE_RESOLUTION << "): " << avg_res << "us";
        // PLOGD_(logger::FileLogger) << "Average time (resolution=1): " << avg_single << "us";

        // PLOGD_(logger::FileLogger) << "End of test...";
        // PLOGD_(logger::FileLogger) << "";
    }


    void testStateAfterSimulations() {
        PLOGD_(logger::FileLogger) << "Running test: RNG state reproducibility.";
        PLOGD_(logger::FileLogger) << "Level: " << level.file_info.nr;

        logger::logBoardState();

        StartGameActions(network.enabled, setup.autorecord, level.random_seed);
        enginehelper::setSimulatorFlag(FALSE);

        // RNG::setInitialRandomBit();
        enginehelper::initZorbristTables();

        // Save starting state
        GameState start_state;
        start_state.setFromEngineState();

        srand(time(NULL));
        int failedComparisons = 0;

        for (int i = 0; i < MAX_SIMULATIONS; i++) {
            GameState referenceState;
            referenceState.setFromEngineState();

            int depth = rand() % MAX_DEPTH + 1;
            std::vector<Action> actionsTaken;

            // Simulate a random number of states forward
            for (int j = 0; j < depth; j++) {
                enginehelper::setEngineRandomPlayerAction();
                actionsTaken.push_back(static_cast<Action>(enginehelper::getEnginePlayerAction()));
                enginehelper::engineSimulate();
            }

            // save forward state
            GameState forwardState;
            forwardState.setFromEngineState();

            // Now restore reference state and perform the same actions
            referenceState.restoreEngineState();
            for (int j = 0; j < depth; j++) {
                // RNG::setSeedEngineHash();
                enginehelper::setEnginePlayerAction(actionsTaken[j]);
                enginehelper::engineSimulate();
            }

            // Compare forward and reference states for equality
            referenceState.setFromEngineState();
            if (!(referenceState == forwardState)) {
                failedComparisons += 1;
                PLOGD_(logger::FileLogger) << "States are not equal.";
                forwardState.restoreEngineState();
                logger::logBoardState();

                referenceState.restoreEngineState();
                logger::logBoardState();

            }
            else {
                PLOGD_(logger::FileLogger) << "States are equal.";
            }
        }

        if (failedComparisons == 0) {
            PLOGD_(logger::FileLogger) << "All states match after simulations";
        }

        PLOGD_(logger::FileLogger) << "End of test...";
        PLOGD_(logger::FileLogger) << "";
    }

}