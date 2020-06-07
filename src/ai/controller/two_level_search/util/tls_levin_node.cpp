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

// Standard Libary/STL
#include <vector>

#include <boost/math/distributions/empirical_cumulative_distribution_function.hpp>

// Includes
#include "tls_distribution.h"
#include "tls_feature.h"
#include "game_state.h"


const int NUM_SAMPLES       = 64;
// const float FEAT_MEAN_0     = 0.0564;
// const float FEAT_MEAN_1     = 0.0224;
// const float FEAT_STDEV_0    = 0.1287;
// const float FEAT_STDEV_1    = 0.1206;
const float FEAT_MEAN_0     = 0.0325;
const float FEAT_MEAN_1     = 0.0197;
const float FEAT_STDEV_0    = 0.1056;
const float FEAT_STDEV_1    = 0.1124;


NodeLevin::NodeLevin(const std::vector<BaseOption*> &path, const std::deque<enginetype::GridCell> &fullGridPath, uint64_t hash, 
              std::size_t pathLength, int numConstraints, int numGems, 
              bool hasDoor, bool hasExpanded, bool wasSkipped, PolicyType policyType, torch::jit::script::Module &module, GameState &initialState) 
: 
    path(path), fullGridPath(fullGridPath), hash(hash), pathLength(pathLength), timesVisited(0), numConstraints(numConstraints),
    combinatorialPartition(CombinatorialPartition(numConstraints)), numGems(numGems), hasDoor(hasDoor), hasExpanded(hasExpanded), wasSkipped(wasSkipped),
    policyType(policyType)
{
    // Only need further initialization if we are using a PyTorch model
    if (policyType == PolicyType::Trivial) {return;}

    // Set device and get feature tensor for this node
    torch::NoGradGuard no_grad_guard;
    torch::Device device = (torch::cuda::is_available()) ? torch::kCUDA : torch::kCPU;
    std::vector<torch::jit::IValue> inputs;
    at::Tensor feature = tlsfeature::getNodeFeature(*this, initialState.Feld_).to(device);
    feature[0].sub_(FEAT_MEAN_0); feature[0].div_(FEAT_STDEV_0);
    feature[1].sub_(FEAT_MEAN_1); feature[1].div_(FEAT_STDEV_1);

    if (policyType == PolicyType::DistNet) {
        // Distnet model outputs the parameters for the distribution directly
        inputs.push_back(torch::stack(feature, 0));
        networkOutput = module.forward(inputs).toTensor();
    }
    else if (policyType == PolicyType::Bayesian) {
        // Bayesian model requires multiple samples to get a sample distribution
        inputs.push_back(torch::stack(feature, 0).repeat({NUM_SAMPLES, 1, 1, 1}));

        at::Tensor model_outputs = module.forward(inputs).toTuple()->elements()[0].toTensor();
        at::Tensor mu = model_outputs.mean();
        at::Tensor var = model_outputs.var();

        networkOutput = torch::stack({mu, var}).reshape({1, 2});
    }
}


/**
 * Get the node policy cost
 * Trivial policy uses 2^(number of constraints) i.e. number of bitwise constraint combinations
 * NN uses the hazard function of the output NN model
 */
double NodeLevin::cost() const {
    if (policyType == PolicyType::DistNet || policyType == PolicyType::Bayesian) {
        return 1.0 / tlsdist::normal_hazard(log((double)timesVisited + 1), networkOutput[0][0].item<double>(), std::sqrt(networkOutput[0][1].item<double>()));
    }
    else {
        return pow(2.0, (double)numConstraints);
    }
}


/**
 * Functor for policy cost comparison duing high-level LevinTS
 */
bool CompareLevinNode::operator() (const NodeLevin &left, const NodeLevin &right) const {
    double costLeft = pow((double)(left.timesVisited + 1), 1.0) * left.cost();
    double costRight = pow((double)(right.timesVisited + 1), 1.0) * right.cost();
    return costLeft < costRight || (costLeft == costRight && left.hash < right.hash);
}