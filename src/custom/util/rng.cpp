

#include "rng.h"

uint64_t RNG::seed_engine = std::chrono::high_resolution_clock::now().time_since_epoch().count();
uint64_t RNG::seed_simulation = std::chrono::high_resolution_clock::now().time_since_epoch().count();
std::mt19937 RNG::gen_engine(seed_engine);
std::mt19937 RNG::gen_simulating(seed_simulation);
std::uniform_int_distribution<uint64_t> RNG::dist_engine(0, MAX_VALUE);
std::uniform_int_distribution<uint64_t> RNG::dist_simulation(0, MAX_VALUE);


/*
 * Set the seed for the RNG used by the engine in gameplay
 * Seed is provided as input
 */
void RNG::setEngineSeed(uint64_t seed) {
    seed_engine = seed;
    gen_engine.seed(seed);
}

/*
 * Set the seed for the RNG used by the engine in gameplay
 * Seed used is current time in microseconds
 */
void RNG::setEngineSeed() {
    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    seed_engine = seed;
    gen_engine.seed(seed);
}


/*
 * Set the seed for the RNG used during search simulation
 * Seed is provided as input
 */
void RNG::setSimulatingSeed(uint64_t seed) {
    seed_simulation = seed;
    gen_simulating.seed(seed);
}

/*
 * Set the seed for the RNG used during search simulation
 * Seed used is current time in microseconds
 */
void RNG::setSimulatingSeed() {
    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    seed_simulation = seed;
    gen_simulating.seed(seed);
}


/*
 *  Get a random number from the RNG, from [0, MAX_VALUE)
 */
uint64_t RNG::getRandomNumber(uint64_t max) {
    uint64_t randnum = (enginehelper::isSimulating() ? dist_simulation(gen_simulating) : dist_engine(gen_engine));
    return randnum % max;
}


/*
 * Get the seed for the RNG used for engine.
 * Normally this is used for saving in replay files to reload games with the same seed
 */
uint64_t RNG::getEngineSeed() {
    return seed_engine;
}


/*
 * Get the seed for the RNG used for simulation.
 * Normally this is used for saving in replay files to reload games with the same seed
 */
uint64_t RNG::getSimulationSeed() {
    return seed_simulation;
}