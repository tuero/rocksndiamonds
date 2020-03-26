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

// Pytorch
#include <torch/torch.h>


// Namespace
using namespace enginehelper;




/**
 * Trivial policy, which is a uniform dist. over total number of constraints,
 * with penality for number of times tried.
 */
bool trivialPolicy(const NodeLevin &left, const NodeLevin &right) {
    double costLeft = (double)(left.timesVisited + 1) * pow(2.0, (double)left.numConstraints);
    double costRight = (double)(right.timesVisited + 1) * pow(2.0, (double)right.numConstraints);

    return costLeft < costRight || (costLeft == costRight && left.hash < right.hash);
}


/**
 * CNN policy
 */
bool distNetPolicy(const NodeLevin &left, const NodeLevin &right) {
    double costLeft = (double)(left.timesVisited + 1) * pow(2.0, (double)left.numConstraints);
    double costRight = (double)(right.timesVisited + 1) * pow(2.0, (double)right.numConstraints);

    return costLeft < costRight || (costLeft == costRight && left.hash < right.hash);
}
