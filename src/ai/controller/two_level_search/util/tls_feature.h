/**
 * @file: tls_feature.h
 *
 * @brief: Construct the features for learning
 * 
 * @author: Jake Tuero
 * Date: February 2020
 * Contact: tuero@ualberta.ca
 */

#ifndef TLS_FEATURE_H
#define TLS_FEATURE_H

// Standard Library/STL
#include <deque>

// Pytorch
#include <torch/torch.h>

// Includes
#include "engine_types.h"
#include "tls_levin_node.h"



namespace tlsfeature {


/**
 * Given a node, find a path of gridcells that that visits each 
 * high level action without constraints.
 * 
 * @param path The high level path
 * @return A deque consisting of the gridcells to travel the high level path
 */
std::deque<enginetype::GridCell> getNodePath(const std::vector<BaseOption*> &path);


/**
 * Get the input feature representation for the levin node
 * 
 * @param node The node to get the feature representation for
 * @param Feld The gameboard to encorporate into the observation
 * @return A 2-channel tensor (2 x width x height) representing the gameboard and highlevel path taken
 */
at::Tensor getNodeFeature(const NodeLevin &node, short Feld[128][128]);

/**
 * Get the observed runtime from the levin node.
 * 
 * @param node The node to get the observation for
 * @param isSolution Flag if this node was a solution (known runtime vs lowerbound)
 * @param wasExhausted Flag is this node had all attempts exhausted and thus will never solve the instance
 * @return A tensor (4) containing the node runtime, and above flags (indicated if exact observed, lowerbound, or exhausted timeout), and if skipped (not enough gems/door)
 */
at::Tensor getNodeObservation(const NodeLevin &node, bool isSolution, bool wasExhausted);

}

#endif