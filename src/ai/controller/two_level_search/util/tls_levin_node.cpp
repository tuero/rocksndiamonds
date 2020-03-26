/**
 * @file: tls_levin_node.cpp
 *
 * @brief: Node for Levin TS
 * 
 * @author: Jake Tuero
 * Date: February 2020
 * Contact: tuero@ualberta.ca
 */


#include "tls_levin_node.h"

// Includes
#include "tls_node_policy.h"


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