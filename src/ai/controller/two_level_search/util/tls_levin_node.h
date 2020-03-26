/**
 * @file: tls_levin_node.h
 *
 * @brief: Node for Levin TS
 * 
 * @author: Jake Tuero
 * Date: February 2020
 * Contact: tuero@ualberta.ca
 */

#ifndef TLS_LEVIN_NODE_H
#define TLS_LEVIN_NODE_H


// Standard Libary/STL
#include <cstdint>              // fixed-width datatypes
#include <vector>
#include <deque>

// Includes
#include "tls_combinatorial_node.h"
#include "base_option.h"
#include "engine_types.h"


/**
 * LevinTS node, contains enough info for policy to determine cost
 */
struct NodeLevin {
    std::vector<BaseOption*> path;
    std::deque<enginetype::GridCell> fullGridPath;
    uint64_t hash;
    std::size_t pathLength;
    std::size_t timesVisited;
    int numConstraints;
    mutable CombinatorialPartition combinatorialPartition;
    int numGems;
    bool hasDoor;
    bool hasExpanded;
    bool wasSkipped;
    // NN dist params
};


/**
 * Functor for policy cost comparison duing high-level LevinTS
 */
struct CompareLevinNode {
    bool operator() (const NodeLevin &left, const NodeLevin &right) const;
};


#endif