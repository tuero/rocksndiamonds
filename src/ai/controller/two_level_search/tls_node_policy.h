/**
 * @file: tls_node_policy.h
 *
 * @brief: Policy for node cost (priority of node ordering in LeveinTS)
 * 
 * @author: Jake Tuero
 * Date: February 2020
 * Contact: tuero@ualberta.ca
 */


// Standard Libary/STL
#include <cstdint>              // fixed-width datatypes

// Includes
#include "util/tls_combinatorial_node.h"


/**
 * LevinTS node, contains enough info for policy to determine cost
 */
struct NodeLevin {
    uint64_t hash;
    int timesVisited;
    int numConstraints;
    mutable CombinatorialPartition combinatorialPartition;
    int numGems;
    bool hasDoor;
};


/**
 * Functor for policy cost comparison duing high-level LevinTS
 */
struct CompareLevinNode {
    bool operator() (const NodeLevin &left, const NodeLevin &right) const;
};