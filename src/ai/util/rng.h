/**
 * @file: rng.h
 *
 * @brief: Random number generator for both engine and simulation for reproducibility.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <random>
#include <algorithm>
#include <chrono>
#include <limits>
#include <cstdint>

#include "../engine/engine_helper.h"

namespace RNG {
    const uint64_t MAX_VALUE = UINT64_MAX;

    /*
     * Set the seed for the RNG used by the engine in gameplay.
     */
    void setEngineSeed(uint64_t seed);

    /*
     * Set the seed to current time in milliseconds for the RNG used by the engine in gameplay.
     */
    void setEngineSeed();

    /*
     * Set the seed from the input for the RNG used by the simulation in planning.
     */
    void setSimulatingSeed(uint64_t seed);

    /*
     * Set the seed to current time in milliseconds for the RNG used by the simulation in planning.
     */
    void setSimulatingSeed();

    /*
     * Get a random number from the engine RNG, from [0, max).
     */
    uint64_t getRandomNumber(uint64_t max = MAX_VALUE);

    /*
     * Get the seed for the RNG used for engine.
     * Normally this is used for saving in replay files to reload games with the same seed
     */
    uint64_t getEngineSeed();

    /*
     * Get the seed for the RNG used for engine.
     * Normally this is used for saving in replay files to reload games with the same seed
     */
    uint64_t getSimulationSeed();
};



#endif  //RANDOMGENERATOR_H