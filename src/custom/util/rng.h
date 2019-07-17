#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <random>
#include <algorithm>
#include <limits>
#include <cstdint>
#include <time.h> 

#include "../engine/engine_helper.h"

class RNG {
private:
    static const uint64_t MAX_VALUE = UINT64_MAX;
    static std::random_device rd;
    static std::mt19937 gen;
    static std::uniform_int_distribution<uint64_t> distribution;
    static uint64_t initialRandomBit;
public:

    /*
     * Set the seed for the RNG
     * This is used to reset the RNG after simulating, so we get
     * reproducible results
     */
    static void setSeed(uint64_t seed);

    /*
     * Set the seed for the RNG from the current state in the engine
     */
    static void setSeedEngineHash();

    /*
     * Sets the intial random bit
     * In order for the environment to have different rollouts during different games,
     * an initial random bit is used in conjunction the state hash for seeding the RNG
     */
    static void setInitialRandomBit();

    /*
     * Get a random number from the RNG, from [0, MAX_VALUE)
     */
    static uint64_t getRandomNumber(uint64_t max = MAX_VALUE);
};



#endif  //RANDOMGENERATOR_H