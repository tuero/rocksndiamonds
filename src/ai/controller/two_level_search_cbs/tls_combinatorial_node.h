/**
 * @file: tls_combinatorial_node.h
 *
 * @brief: Incrementally generates all binary combinations (ordered by binomial coefficient complexity)
 * 
 * @author: Jake Tuero
 * Date: January 2020
 * Contact: tuero@ualberta.ca
 */


// Standard Libary/STL
#include <vector>
#include <cstdint>


/**
 * Start with list of number of constraints, from a high level path (each node on path has a constraint size).
 * For a given constraint size, all binary combination of the constraints need to be checked. For the whole 
 * high level path, the Cartesian product of all these binary combinations need to be checked.
 * 
 * For a given constraint size of n, we pair i and n-i for i \in [0, n]. The idea is that we should check 
 * none of the constraints (0) and all the constraints (10-0) together, 1 of each constraint (1) and all
 * but one constraint (10-1) together, etc. This is ordering by (n choose k) = (n choose n-k) which is the measure
 * of description length complexity.
 * 
 * If a single high level node 'i' has n_i constraints, there will be floor(n_i/2)+1 of these pairs (with the possibility of
 * even number of constraints n_i having a final pair with only 1 number (the middle element)). These are arranged in a 
 * list L_i= [{0, n_i}, {1, n_i-1}, ...] with |L_i|=floor(n_i)/2)+1.
 * 
 * The Cartesian product of all lists L_i (1 for each node in the high level path) needs to be checked. Since each L_i
 * is ordered by description complexity, we want to first try the first element of each L_i, the first element of each 
 * L_i except the 2nd element from one of the L_i's, and so on, until we reach the last element of each of the L_i's.
 * 
 * In the example of  the first element of each L_i except the 2nd element from one of the L_i's, since we are indifferent as
 * to which L_i has the 2nd element selected, we can cleverly group these as the partitions of N, starting from 0 and ending at
 * sum(|L_i|). A partition represents the index used for each L_i. For example, suppose the high level path has constraints (4, 3). 
 * The first partition is of 0, which represents the indices for L_1, and L_2 as (0,0). The second partition is of 1, which represents
 * the indices (1,0) and (0,1). The third partition is of 2, which represents the indices (2,0), (1,1), (0,2) and so on.
 * 
 * These partitions are selected by increasing N. A given partition represents the index in the list of (n choose k) pairs. 
 * For a given partition (p_1, p_2, ..., p_k), it represents the number of constraints ({p_1, n_1-p_1}, ..., {p_k, n_k-p_k})
 * For each number of constraints, all binary numbers of length n_i with p_i bits set AND (n_i-p_i) it set are added to a list.
 * Each binary number represents which constraints are to be set. The Cartesian product of these binary numbers (set for each high level
 * node for the given current partition) needs to be individually tried.
 */
// class CombinatorialPartition {
// private:
//     std::vector<int> numConstraintsOptionPair_;     // Input from the user which determines binary bits to be generated
//     bool resetFlag;

//     int currentSumToDistribute_;    // Current sum to partition between option pairs
//     int maxSumToDistribute_;        // Max sum to partition between option pairs
//     int totalConstraintCount_;      // Total constraints for path, used for new constraint detection
//     std::vector<int> maxSumPerOptionPair_;       // Max sum to partition for each option pair
//     int currentPartitionIdx_;       // Current index of currentPartitionIndices_
//     std::vector<std::vector<int>> currentPartitionIndices_;  // List of partitions for the current partition sum
    
//     int currentBitIndex_;       // Current Cartesian product index of items in constraintByBits_
//     int maxBitIndex_;           // Max Cartesian product index of items in constraintByBits_
//     std::vector<std::vector<uint64_t>> constraintByBits_;     // Bits representing constraint for current partition sum


//     /**
//      * Check if the current Cartesian Product of binary bits per constraint is complete for the given partition.
//      * 
//      * @return True if the current partition is complete. 
//      */
//     bool currentBitComplete() const;

//     /**
//      * Check all partitions of the current sum have been exhausted.
//      * 
//      * @return True if all partitions of the current sum have been exhausted.
//      */
//     bool currentPartitionSumComplete() const;

//     /**
//      * Check if the all sums of the partitions have been exhausted.
//      * 
//      * @return True if the all partitions have been exhausted.
//      */
//     bool isSumComplete() const;

//     /**
//      *  Set the internal sets of binary combinations (for each constraint set) for the given partition.
//      * 
//      * @param partitionIndices The index of binomial coefficents for the given partition (which uses the current partition sum)
//      */
//     void setConstraintByBits(std::vector<int> &partitionIndices);

//     /**
//      * Set the internal sets of binary combinations (for each constraint set) for the given partition.
//      * 
//      * @param partitionIndices The index of binomial coefficents for the given partition (which uses the current partition sum)
//      */
//     void updateConstraintByBits();

// public:

//     CombinatorialPartition();

//     /**
//      * Check if the given constraint counts exceeds the currently saved counts.
//      * 
//      * @param numConstraintsOptionPair The vector of constraint pair counts.
//      * @return True if a reset is required, false otherwise.
//      */
//     bool requiresReset(std::vector<int> &numConstraintsOptionPair);

//     /**
//      * Check if all binary combinations for the input constraint list have been exhausted.
//      * This is true when currentBitComplete, currentPartitionSumComplete, and isSumComplete are true.
//      * 
//      * @return True if all binary combinations have been returned.
//      */
//     bool isComplete() const;

//     /**
//      * Reset the combinatorics back to to the beginning as a new list of constraint counts is given.
//      * 
//      * @param numConstraintsOptionPair The vector of constraint pair counts.
//      */
//     void reset(std::vector<int> &numConstraintsOptionPair);

//     /**
//      * Get the next list of binary integers (1 for each set of constraints in the high level path)
//      * 
//      * @return A list of bits which represents the constraints to be set for each node in the high level path.
//      */
//     std::vector<uint64_t> getNextConstraintBits();
// };


class CombinatorialPartition {
public:
    bool resetFlag;

    
        
    uint64_t counter_;
    uint64_t maxCounter_;
    uint64_t currentBit_;
    int totalConstraintCount_;       // Total constraints for path, used for new constraint detection
    int currentSumToDistribute_;     // Current sum to partition between option pairs
    int maxSumToDistribute_;

    CombinatorialPartition();

    bool requiresReset(std::vector<int> &numConstraintsOptionPair);

    bool requiresReset(int totalConstraintCount);

    bool isComplete();


    void reset(std::vector<int> numConstraintsOptionPair);
    


    uint64_t getNextConstraintBits();
};