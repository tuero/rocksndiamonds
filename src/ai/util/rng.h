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


#include <cstdint>

#include "../engine/engine_helper.h"

namespace RNG {
    const uint64_t MAX_VALUE = UINT64_MAX;

    /**
     * Set the seed for the RNG used by the engine in gameplay.
     * 
     * @param seed Input seed for the game engine RNG.
     */
    void setEngineSeed(uint64_t seed);

    /**
     * Set the seed to current time in milliseconds for the RNG used by the engine in gameplay.
     */
    void setEngineSeed();

    /**
     * Reset the engine seed to the stored engine seed.
     */
    void resetToEngineSeed();

    /**
     * Set the seed from the input for the RNG used by the simulation in planning.
     * 
     * @param seed Input seed for the game engine RNG.
     */
    void setSimulatingSeed(uint64_t seed);

    /**
     * Set the seed to current time in milliseconds for the RNG used by the simulation in planning.
     */
    void setSimulatingSeed();

    /**
     * Reset the simulation seed to the stored simulation seed.
     */
    void resetToSimulationSeed();

    /**
     * Get a random number from the engine RNG, from [0, max).
     * 
     * @param max The endpoint for the range to draw from.
     */
    uint64_t getRandomNumber(uint64_t max = MAX_VALUE);

    /**
     * Get the seed for the RNG used for engine.
     * Normally this is used for saving in replay files to reload games with the same seed.
     * 
     * @return The seed used for the Engine RNG.
     */
    uint64_t getEngineSeed();

    /**
     * Get the seed for the RNG used for engine.
     * Normally this is used for saving in replay files to reload games with the same seed.
     * 
     * @return The seed used for the Simulation RNG.
     */
    uint64_t getSimulationSeed();
};



#endif  //RANDOMGENERATOR_H