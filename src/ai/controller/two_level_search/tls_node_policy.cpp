/**
 * @file: tls_node_policy.cpp
 *
 * @brief: Policy for node cost (priority of node ordering in LeveinTS)
 * 
 * @author: Jake Tuero
 * Date: February 2020
 * Contact: tuero@ualberta.ca
 */

#include "tls_node_policy.h"

// Standard Libary/STL
#include <cmath>


/**
 * Trivial policy, which is a uniform dist. over total number of constraints,
 * with penality for number of times tried.
 */
bool trivialPolicy(const NodeLevin &left, const NodeLevin &right) {
    double costLeft = (double)left.timesVisited * pow(2.0, (double)left.numConstraints);
    double costRight = (double)right.timesVisited * pow(2.0, (double)right.numConstraints);

    return costLeft < costRight || (costLeft == costRight && left.hash < right.hash);
}


bool distNetPolicy(const NodeLevin &left, const NodeLevin &right) {
    double costLeft = (double)left.timesVisited * pow(2.0, (double)left.numConstraints);
    double costRight = (double)right.timesVisited * pow(2.0, (double)right.numConstraints);

    return costLeft < costRight || (costLeft == costRight && left.hash < right.hash);
}


/**
 * Functor for policy cost comparison duing high-level LevinTS
 */
bool CompareLevinNode::operator() (const NodeLevin &left, const NodeLevin &right) const {
    #ifdef TRIVIAL_POLICY
        return trivialPolicy(left, right);
    #else
        return distNetPolicy(left, right);
    #endif
}