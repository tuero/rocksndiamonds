/**
 * @file: tls_combinatorial_node.cpp
 *
 * @brief: Incrementally generates all binary combinations (ordered by binomial coefficient complexity)
 * 
 * @author: Jake Tuero
 * Date: January 2020
 * Contact: tuero@ualberta.ca
 */

#include "tls_combinatorial_node.h"

// Standard Libary/STL
#include <cstdint>
#include <algorithm>
#include <numeric>

// Includes
#include "logger.h"


// ------------------------------------------------------------------------------------
uint64_t nextBinaryString(int numBitsSet, uint64_t current) {

    // number of bits set is zero
    if (numBitsSet == 0) {return 0;}
#ifdef __GNUC__
    uint64_t t = (current | (current - 1));
    return (t + 1) | (((~t & -~t) - 1) >> (__builtin_ctz(current) + 1));
#else
    uint64_t t = (current | (current - 1)) + 1;
    return t | ((((t & -t) / (current & -current)) >> 1) - 1);
#endif
}

/**
 * https://stackoverflow.com/questions/9330915/number-of-combinations-n-choose-r-in-c
 */
uint64_t nChoosek(uint64_t n, uint64_t k) {
    if (k > n) {return 0;}
    if (k * 2 > n) {k = n-k;}
    if (k == 0) {return 1;}

    uint64_t result = n;
    for(uint64_t i = 2; i <= k; ++i) {
        result *= (n-i+1);
        result /= i;
    }
    return result;
}


namespace tlsbits {

/**
 * Given a Combinatorial Partition, gets the next bit pattern.
 * 
 * Ordering of the bits are by combinatorial complexity i.e. (n choose k)
 */
uint64_t getNextConstraintBits(CombinatorialPartition &cPartition)
{
    // return cPartition.currentBit++;
    // Done current sum to distribute
    if (cPartition.counter == cPartition.maxCounter) {
        cPartition.counter = 0;
        cPartition.maxCounter = 2 * nChoosek(cPartition.totalConstraintCount, ++cPartition.currentSumToDistribute);
        cPartition.currentBit = ((uint64_t)1 << cPartition.currentSumToDistribute) - 1;
    }

    // Both current bit and compliment sent, increment to next lexicographical ordering
    if (cPartition.counter % 2 == 0 && cPartition.counter > 0) {
        cPartition.currentBit = nextBinaryString(cPartition.currentSumToDistribute, cPartition.currentBit);
    }

    // Send bit and save the complement to send for next time
    uint64_t bitsToSend = cPartition.currentBit;
    cPartition.currentBit = ~cPartition.currentBit;
    ++cPartition.counter;

    // 
    if (cPartition.currentSumToDistribute == cPartition.maxSumToDistribute && cPartition.totalConstraintCount % 2 == 0) {
        cPartition.currentBit = ~cPartition.currentBit;
        ++cPartition.counter;
    }
    return bitsToSend;
}

} // namespace tlsbits