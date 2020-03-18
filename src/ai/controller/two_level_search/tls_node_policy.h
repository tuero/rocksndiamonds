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
#include <unordered_map>
#include <unordered_set>

// Includes
#include "base_option.h"
#include "util/tls_combinatorial_node.h"


/**
 * LevinTS node, contains enough info for policy to determine cost
 */
struct NodeLevin {
    std::vector<BaseOption*> path;
    uint64_t hash;
    std::size_t pathLength;
    int timesVisited;
    int numConstraints;
    mutable CombinatorialPartition combinatorialPartition;
    int numGems;
    bool hasDoor;
    // NN dist params
};


/**
 * Functor for policy cost comparison duing high-level LevinTS
 */
struct CompareLevinNode {
    bool operator() (const NodeLevin &left, const NodeLevin &right) const;
};