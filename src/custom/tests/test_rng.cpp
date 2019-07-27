

#include "test_rng.h"


namespace testrng{

    void testStateAfterSimulations() {
        PLOGD_(logwrap::FileLogger) << "Running test: RNG state reproducibility.";
        PLOGD_(logwrap::FileLogger) << "Level: " << level.file_info.nr;

        logwrap::logBoardState();

        StartGameActions(network.enabled, setup.autorecord, level.random_seed);
        enginehelper::setSimulatorFlag(FALSE);

        // RNG::setInitialRandomBit();
        enginehelper::initZorbristTables();

        // Save starting state
        GameState start_state;
        start_state.setFromSimulator();

        srand(time(NULL));
        int failedComparisons = 0;

        for (int i = 0; i < MAX_SIMULATIONS; i++) {
            GameState referenceState;
            referenceState.setFromSimulator();

            int depth = rand() % MAX_DEPTH + 1;
            std::vector<Action> actionsTaken;

            // Simulate a random number of states forward
            for (int j = 0; j < depth; j++) {
                // RNG::setSeedEngineHash();
                enginehelper::setEngineRandomPlayerAction();
                actionsTaken.push_back(static_cast<Action>(enginehelper::getEnginePlayerAction()));
                enginehelper::engineSimulate();
            }

            // save forward state
            GameState forwardState;
            forwardState.setFromSimulator();

            // Now restore reference state and perform the same actions
            referenceState.restoreSimulator();
            for (int j = 0; j < depth; j++) {
                // RNG::setSeedEngineHash();
                enginehelper::setEnginePlayerAction(actionsTaken[j]);
                enginehelper::engineSimulate();
            }

            // Compare forward and reference states for equality
            referenceState.setFromSimulator();
            if (!(referenceState == forwardState)) {
                failedComparisons += 1;
                PLOGD_(logwrap::FileLogger) << "States are not equal.";
                forwardState.restoreSimulator();
                logwrap::logBoardState();

                referenceState.restoreSimulator();
                logwrap::logBoardState();

            }
            else {
                PLOGD_(logwrap::FileLogger) << "States are equal.";
            }
        }

        if (failedComparisons == 0) {
            PLOGD_(logwrap::FileLogger) << "All states match after simulations";
        }

        PLOGD_(logwrap::FileLogger) << "End of test...";
        PLOGD_(logwrap::FileLogger) << "";
    }

}