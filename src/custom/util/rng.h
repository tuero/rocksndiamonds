#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <random>
#include <algorithm>
#include <chrono>
#include <limits>
#include <cstdint>

#include "../engine/engine_helper.h"

class RNG {
private:
    static const uint64_t MAX_VALUE = UINT64_MAX;
    static std::mt19937 gen_engine;
    static std::mt19937 gen_simulating;
    static std::uniform_int_distribution<uint64_t> dist_engine;
    static std::uniform_int_distribution<uint64_t> dist_simulation;
    static uint64_t seed_engine;
    static uint64_t seed_simulation;
public:

    /*
     * Set the seed for the RNG used by the engine in gameplay
     * Seed is provided as input
     */
    static void setEngineSeed(uint64_t seed);

    /*
     * Set the seed for the RNG used by the engine in gameplay
     * Seed used is current time in microseconds
     */
    static void setEngineSeed();

    /*
     * Set the seed for the RNG used during search simulation
     * Seed is provided as input
     */
    static void setSimulatingSeed(uint64_t seed);

    /*
     * Set the seed for the RNG used during search simulation
     * Seed used is current time in microseconds
     */
    static void setSimulatingSeed();

    /*
     * Get a random number from the RNG, from [0, MAX_VALUE)
     */
    static uint64_t getRandomNumber(uint64_t max = MAX_VALUE);

    /*
     * Get the seed for the RNG used for engine.
     * Normally this is used for saving in replay files to reload games with the same seed
     */
    static uint64_t getEngineSeed();

    /*
     * Get the seed for the RNG used for engine.
     * Normally this is used for saving in replay files to reload games with the same seed
     */
    static uint64_t getSimulationSeed();
};



#endif  //RANDOMGENERATOR_H