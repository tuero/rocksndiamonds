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


// /**
//  * Generate all binary string numbers of size numBitsLength with numBitsSet set to 1.
//  * See: https://stackoverflow.com/questions/1851134/generate-all-binary-strings-of-length-n-with-k-bits-set
//  * @param numBitsSet Number of bits that are set
//  * @param numBitsLength Length of the binary string
//  * @return Vector of all binary 'string' numbers satisfying the above.
//  */
// std::vector<uint64_t> allBinaryString(int numBitsSet, int numBitsLength) {

//     // number of bits set is zero
//     if (numBitsSet == 0) {return {0};}
    
//     uint64_t max = ((uint64_t)1 << numBitsLength) - 1;
//     uint64_t prev = ((uint64_t)1 << numBitsSet) - 1;
//     uint64_t curr = prev;

//     // number of bits set is equal to bit length
//     if (curr == max) {return {max};}

//     std::vector<uint64_t> allOrderings;
//     while (curr < max) {
//         prev = curr;
//         uint64_t t = (prev | (prev - 1)) + 1;
//         curr = t | ((((t & -t) / (prev & -prev)) >> 1) - 1);
//         allOrderings.push_back(prev);
//     }

//     return allOrderings;
// }


// /**
//  * Hash for a given permutation.
//  */
// uint64_t _HeapsVecToHash(std::vector<int> &permutation) {
//     uint64_t hash = 0;
//     for (auto const & p : permutation) {
//         hash = (hash * 100) + p;
//     }
//     return hash;
// }


// /**
//  * Heaps Algorithm
//  * Recursively find all permutations of a given partition.
//  * 
//  * @param k Recursion height
//  * @param current The current permutation
//  * @param seenPermutations Previously seen permutations, used to not include duplicates (0001 has multiple permutations of reordering
//  *  the zeros, which we don't care for)
//  * @param permutations Reference to externam container to store new found permutations 
//  */
// void _HeapsAlgorithm(int k, std::vector<int> &current, std::unordered_set<int> &seenPermutations, std::vector<std::vector<int>> &permutations) {
//     if (k == 1) {
//         uint64_t hash = _HeapsVecToHash(current);
//         if (seenPermutations.find(hash) == seenPermutations.end()) {
//             permutations.push_back(current);
//             seenPermutations.insert(hash);
//         }
//         return;
//     }

//     _HeapsAlgorithm(k-1, current, seenPermutations, permutations);
//     for (int i = 0; i < k-1; i++) {
//         if (k % 2 == 0) {
//             std::iter_swap(current.begin() + i, current.begin() + (k-1));
//         }
//         else {
//             std::iter_swap(current.begin(), current.begin() + (k-1));
//         }
//         _HeapsAlgorithm(k-1, current, seenPermutations, permutations);
//     }
// }


// /**
//  * Generate all permutations of a given partition.
//  * This uses Heap's algorithm.
//  * 
//  * @param startingPermutation The starting permutation.
//  * @return Vector of all permutations for the given partition.
//  */
// std::vector<std::vector<int>> generateAllPermutations(std::vector<int> &startingPermutation) {
//     std::vector<std::vector<int>> allOrderings;
//     std::unordered_set<int> seenPermutations;
//     _HeapsAlgorithm(startingPermutation.size(), startingPermutation, seenPermutations, allOrderings);
//     return allOrderings;
// }


// /**
//  * Generate all orderings of a given partition in Colexigraphic ordering (reading right to left).
//  * Next index found to increment, everything left of index (except start) gets set to updated index value,
//  * and start gets the remaining sum to eventually redistributed back to the right.
//  */
// bool _nextPartitionColexigraphicOrdering(int n, std::vector<int> &currentPartition) {
//     int index = -1;
    
//     // Find left most index to decrement
//     for (int i = 0; i < (int)currentPartition.size(); i++) {
//         if (currentPartition[i] < currentPartition[0] - 1) {index = i; break;}
//     }

//     // Final Colexigraphic order reached, return
//     if (index == -1) {return false;}

//     // Index gets incremented by 1 
//     // Left of index decreases (except start) gets decremented to [index] + 1
//     for (int j = 1; j < index + 1; j++) {
//         currentPartition[j] = currentPartition[index] + 1;
//     }

//     // Remaining amount gets added back to the beginning.
//     currentPartition[0] = n - std::accumulate(currentPartition.begin()+1, currentPartition.end(), 0);
//     return true;
// }

// /**
//  * Find all partitions of n. Partition of n are partitions of non-negative integers which sum to n.
//  * E. W. Dijkstra, A Discipline of Programming, Prentice-Hall, 1997 [FACSIMILE]
//  * 
//  * @param sumToDistribute The sum to partition.
//  * @param maxPermutationValues List of max values allowed for each permutation of a given partition.
//  * @return Vector of all valid partitions (along with permutations) of a given integer.
//  */
// std::vector<std::vector<int>> findAllPartitions(int sumToDistribute, std::vector<int> &maxPermutationValues) {
//     std::vector<int> currentPartition(maxPermutationValues.size(), 0);
//     currentPartition[0] = sumToDistribute;
    
//     std::vector<std::vector<int>> result{currentPartition};
//     std::vector<std::vector<int>> output;

//     // Generate all partitions in Colexigraphic ordering
//     while (_nextPartitionColexigraphicOrdering(sumToDistribute, currentPartition)) {
//         result.push_back(currentPartition);
//     }

//     auto zipCompareUpperBound = [](std::vector<int> &current, std::vector<int> &upperBound) {
//         for (int i = 0; i < (int)current.size(); i++) {if (current[i] >= upperBound[i]) {return false;}} return true;
//     };

//     // For each partition, we need to generate all permutations (as we get {3,1} but also need to consider {1,3})
//     for (auto & partition : result) {
//         for (auto & order : generateAllPermutations(partition)) {
//             // We only care about the specific permutation if the values are less than the maximum allowed permutation values
//             if (zipCompareUpperBound(order, maxPermutationValues)) {
//                 output.push_back(order);
//             }
//         }
//     }
//     return output;
// }


// CombinatorialPartition::CombinatorialPartition() {
//     std::vector<int> defaultConstraints{0};
//     reset(defaultConstraints);
// }


// /**
//  * Check if the current Cartesian Product of binary bits per constraint is complete for the given partition.
//  */
// bool CombinatorialPartition::currentBitComplete() const {
//     return currentBitIndex_ == maxBitIndex_;
// }


// /**
//  * Check all partitions of the current sum have been exhausted.
//  */
// bool CombinatorialPartition::currentPartitionSumComplete() const {
//     return currentPartitionIdx_ == (int)currentPartitionIndices_.size();
// }


// /**
//  * Check if the all sums of the partitions have been exhausted.
//  */
// bool CombinatorialPartition::isSumComplete() const {
//     return currentSumToDistribute_ == maxSumToDistribute_;
// }


// /**
//  * Set the internal sets of binary combinations (for each constraint set) for the given partition.
//  */
// void CombinatorialPartition::setConstraintByBits(std::vector<int> &partitionIndices) {
//     currentBitIndex_ = 0;
//     maxBitIndex_ = 1;
//     constraintByBits_.clear();

//     // For each partition, index, we consider both (n, k) and (n-k, k)
//     for (int i = 0; i < (int)numConstraintsOptionPair_.size(); i++) {
//         int lowConstraints = partitionIndices[i];
//         int highConstraints = numConstraintsOptionPair_[i] - partitionIndices[i];

//         std::vector<uint64_t> bits = allBinaryString(lowConstraints, numConstraintsOptionPair_[i]);

//         // If number of constraints was odd, these will equal, and
//         // the resulting bits complement would equal the original bits
//         if (lowConstraints != highConstraints) {
//             std::vector<uint64_t> bitsCompliment = allBinaryString(highConstraints, numConstraintsOptionPair_[i]);
//             bits.insert(bits.end(), bitsCompliment.begin(), bitsCompliment.end()); 
//         }

//         maxBitIndex_ *= bits.size();
//         constraintByBits_.push_back(bits);
//     }
// }

// /**
//  * Set the internal sets of binary combinations (for each constraint set) for the given partition.
//  */
// void CombinatorialPartition::updateConstraintByBits() {
//     // If current partition set is complete, we increment sum to distribute and get next list of partitions
//     if (currentPartitionSumComplete() || !resetFlag) {
//         currentPartitionIndices_ = findAllPartitions(++currentSumToDistribute_, maxSumPerOptionPair_);
//         currentPartitionIdx_ = 0;
//     }

//     // Get bits for next partition
//     setConstraintByBits(currentPartitionIndices_[currentPartitionIdx_++]);
// }


// /**
//  * Check if the given constraint counts exceeds the currently saved counts.
//  */
// bool CombinatorialPartition::requiresReset(std::vector<int> &numConstraintsOptionPair) {
//     return std::accumulate(numConstraintsOptionPair.begin(), numConstraintsOptionPair.end(), 0) > totalConstraintCount_;
// }


// /**
//  * Check if all binary combinations for the input constraint list have been exhausted.
//  */
// bool CombinatorialPartition::isComplete() const {
//     return isSumComplete() && currentPartitionSumComplete() && currentBitComplete() && resetFlag;
// }


// /**
//  * Reset the combinatorics back to to the beginning as a new list of constraint counts is given.
//  */
// void CombinatorialPartition::reset(std::vector<int> &numConstraintsOptionPair) {
//     resetFlag = false;
//     numConstraintsOptionPair_ = numConstraintsOptionPair;
//     currentSumToDistribute_ = -1;
//     maxSumToDistribute_ = 0;
//     currentPartitionIdx_ = 0;
//     currentPartitionIndices_.empty();
//     totalConstraintCount_ = std::accumulate(numConstraintsOptionPair_.begin(), numConstraintsOptionPair_.end(), 0);
    
//     // Set the max sum to partition for each options pair
//     // This represents an index for a (k, n-k) pair.
//     maxSumPerOptionPair_.clear();
//     for (auto const & numConstraints : numConstraintsOptionPair_) {
//         int partitionLength = (numConstraints / 2) + 1;
//         maxSumPerOptionPair_.push_back(partitionLength);
//         maxSumToDistribute_ += partitionLength - 1;
//     }

//     // Set the partitions for the current sum
//     updateConstraintByBits();
// }


// /**
//  * Get the next list of binary integers (1 for each set of constraints in the high level path)
//  */
// std::vector<uint64_t> CombinatorialPartition::getNextConstraintBits() {
//     resetFlag = true;
//     std::vector<uint64_t> bits;

//     // If Cartesian Product of current bits already sent, query next sets
//     if (currentBitComplete()) {
//         updateConstraintByBits();
//     }
    
//     int tempIndex = currentBitIndex_++;
//     // For each bit option, index represents index of Cartesian product
//     for (auto const & bitsForConstraint : constraintByBits_) {
//         bits.push_back(bitsForConstraint[tempIndex % bitsForConstraint.size()]);
//         tempIndex /= bitsForConstraint.size();
//     }
//     return bits;
// }



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