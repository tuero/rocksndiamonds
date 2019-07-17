

#include "rng.h"

std::random_device RNG::rd;
std::mt19937 RNG::gen(rd());
std::uniform_int_distribution<uint64_t> RNG::distribution(0, MAX_VALUE);
uint64_t RNG::initialRandomBit = 0;


/*
 * Set the seed for the RNG
 * This is used to reset the RNG after simulating, so we get
 * reproducible results
 */
void RNG::setSeed(uint64_t seed) {
    gen.seed(seed);
}


/*
 * Set the seed for the RNG from the current state in the engine
 */
void RNG::setSeedEngineHash() {
    gen.seed(enginehelper::stateToHash());
}


/*
 * Sets the intial random bit
 * In order for the environment to have different rollouts during different games,
 * an initial random bit is used in conjunction the state hash for seeding the RNG
 */
void RNG::setInitialRandomBit() {
    initialRandomBit = time(NULL);
}


/*
 *  Get a random number from the RNG, from [0, MAX_VALUE)
 */
uint64_t RNG::getRandomNumber(uint64_t max) {
    return (distribution(gen) + initialRandomBit) % max;
}