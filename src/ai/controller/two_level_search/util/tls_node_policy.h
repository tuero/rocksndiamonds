/**
 * @file: tls_node_policy.h
 *
 * @brief: Policy for node cost (priority of node ordering in LeveinTS)
 * 
 * @author: Jake Tuero
 * Date: February 2020
 * Contact: tuero@ualberta.ca
 */

#ifndef TLS_NODE_POLICY_H
#define TLS_NODE_POLICY_H


// Includes
#include "tls_levin_node.h"


/**
 * Trivial policy, which is a uniform dist. over total number of constraints,
 * with penality for number of times tried.
 * 
 * @param left Left levin node to compare
 * @param right Right levin node to compare
 * @return True if left < right 
 */
bool trivialPolicy(const NodeLevin &left, const NodeLevin &right);


/**
 * CNN policy
 * 
 * @param left Left levin node to compare
 * @param right Right levin node to compare
 * @return True if left < right 
 */
bool distNetPolicy(const NodeLevin &left, const NodeLevin &right);

#endif