

#include "test_rng.h"


namespace testrng{

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