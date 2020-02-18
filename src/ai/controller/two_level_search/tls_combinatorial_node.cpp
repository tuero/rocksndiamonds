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
#include <vector>
#include <cstdint>
#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <iostream>

// Includes
#include "logger.h"


// ------------------------------------------------------------------------------------
uint64_t nextBinaryString(int numBitsSet, int numBitsLength, uint64_t current) {

    // number of bits set is zero
    if (numBitsSet == 0) {return {0};}
    uint64_t t = (current | (current - 1)) + 1;
#ifdef __GNUC__
    return (t + 1) | (((~t & -~t) - 1) >> (__builtin_ctz(current) + 1));
#else
    return t | ((((t & -t) / (current & -current)) >> 1) - 1);
#endif
}

uint64_t nChoosek(uint64_t n, uint64_t k) {
    if (k > n) {return 0;}
    if (k * 2 > n) {k = n-k;}
    if (k == 0) {return 1;}

    uint64_t result = n;
    for( int i = 2; i <= k; ++i ) {
        result *= (n-i+1);
        result /= i;
    }
    return result;
}

CombinatorialPartition::CombinatorialPartition() {
    std::vector<int> temp{0};
    reset(temp);
}

bool CombinatorialPartition::requiresReset(std::vector<int> &numConstraintsOptionPair) {
    return std::accumulate(numConstraintsOptionPair.begin(), numConstraintsOptionPair.end(), 0) > totalConstraintCount_;
}

bool CombinatorialPartition::requiresReset(int totalConstraintCount) {
    return totalConstraintCount > totalConstraintCount_;
}

bool CombinatorialPartition::isComplete() {
    return currentSumToDistribute_ == maxSumToDistribute_  && counter_ == maxCounter_;
}


void CombinatorialPartition::reset(std::vector<int> numConstraintsOptionPair) {
    resetFlag = false;
    totalConstraintCount_ = std::accumulate(numConstraintsOptionPair.begin(), numConstraintsOptionPair.end(), 0);
    maxSumToDistribute_ = (totalConstraintCount_ / 2);
    currentSumToDistribute_ = -1;
    currentBit_ = 0;
    counter_ = -1;
    maxCounter_ = -1;
}


void CombinatorialPartition::reset(int totalConstraintCount) {
    resetFlag = false;
    totalConstraintCount_ = totalConstraintCount;
    maxSumToDistribute_ = (totalConstraintCount_ / 2);
    currentSumToDistribute_ = -1;
    currentBit_ = 0;
    counter_ = -1;
    maxCounter_ = -1;
}


uint64_t CombinatorialPartition::getNextConstraintBits() {
    resetFlag = true;

    // Done current sum to distribute
    if (counter_ == maxCounter_) {
        counter_ = 0;
        maxCounter_ = 2 * nChoosek(totalConstraintCount_, ++currentSumToDistribute_);
        currentBit_ = ((uint64_t)1 << currentSumToDistribute_) - 1;
    }

    // Both current bit and compliment sent, increment to next lexicographical ordering
    if (counter_ % 2 == 0 && counter_ > 0) {
        currentBit_ = nextBinaryString(currentSumToDistribute_, totalConstraintCount_, currentBit_);
    }
    uint64_t bitsToSend = currentBit_;
    currentBit_ = ~currentBit_;
    ++counter_;
    if (currentSumToDistribute_ == maxSumToDistribute_ && totalConstraintCount_ % 2 == 0) {
        currentBit_ = ~currentBit_;
        ++counter_;
    }
    return bitsToSend;
}