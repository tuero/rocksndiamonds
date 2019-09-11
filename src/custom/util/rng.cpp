/**
 * @file: rng.h
 *
 * @brief: Random number generator for both engine and simulation for reproducibility.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "rng.h"

namespace RNG {

uint64_t seedEngine_ = std::chrono::high_resolution_clock::now().time_since_epoch().count();
uint64_t seedSimulation_ = std::chrono::high_resolution_clock::now().time_since_epoch().count();

// Generators
std::mt19937 genEngine_(seedEngine_);
std::mt19937 genSimulating_(seedSimulation_);

// Uniform distributions for generator
std::uniform_int_distribution<uint64_t> distEngine_(0, MAX_VALUE);
std::uniform_int_distribution<uint64_t> distSimulation_(0, MAX_VALUE);


/*
 * Set the seed from the input for the RNG used by the engine in gameplay.
 * 
 * @param seed Input seed for the game engine RNG.
 */
void setEngineSeed(uint64_t seed) {
    seedEngine_ = seed;
    genEngine_.seed(seed);
}


/*
 * Set the seed to current time in milliseconds for the RNG used by the engine in gameplay.
 */
void setEngineSeed() {
    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    seedEngine_ = seed;
    genEngine_.seed(seed);
}


/*
 * Set the seed from the input for the RNG used by the simulation in planning.
 * 
 * @param seed Input seed for the game engine RNG.
 */
void setSimulatingSeed(uint64_t seed) {
    seedSimulation_ = seed;
    genSimulating_.seed(seed);
}

/*
 * Set the seed to current time in milliseconds for the RNG used by the simulation in planning.
 */
void setSimulatingSeed() {
    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    seedSimulation_ = seed;
    genSimulating_.seed(seed);
}


/*
 * Get a random number from the engine RNG, from [0, max).
 * 
 * @param max The endpoint for the range to draw from.
 */
uint64_t getRandomNumber(uint64_t max) {
    uint64_t randnum = (enginehelper::isSimulating() ? distSimulation_(genSimulating_) : distEngine_(genEngine_));
    return randnum % max;
}


/*
 * Get the seed for the RNG used for engine.
 * Normally this is used for saving in replay files to reload games with the same seed
 */
uint64_t getEngineSeed() {
    return seedEngine_;
}


/*
 * Get the seed for the RNG used for simulation.
 * Normally this is used for saving in replay files to reload games with the same seed
 */
uint64_t getSimulationSeed() {
    return seedSimulation_;
}

} // namespace RNG