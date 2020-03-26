/**
 * @file: tls_combinatorial_node_test.cpp
 *
 * @brief: Test functions for combinatorial node helper function for controller TLS
 * @author: Jake Tuero
 * Date: March 2020
 * Contact: tuero@ualberta.ca
 */


// Testing framework
#include <catch2/catch.hpp>

// Standard Libary/STL
#include <unordered_map>

// Includes
#include "../../controller/two_level_search/util/tls_combinatorial_node.h"


TEST_CASE("TLS check all combinatorial bit patterns", "[tls_combinatorial_node]") {
    std::vector<uint64_t> numConstraintsOptionPair = {6, 6, 6, 6};
    uint64_t sum = std::accumulate(numConstraintsOptionPair.begin(), numConstraintsOptionPair.end(), 0);
    CombinatorialPartition combinatorialPartition(sum);

    // Set false flags for seen bitpatterns
    std::unordered_map<uint64_t, bool> flags;
    for (uint64_t i = 0; i <= (1ULL << sum) - 1; i++) {
        flags[i] = false;
    }

    // Get all bitpatterns  
    while(!combinatorialPartition.isComplete()) {
        uint64_t bits = tlsbits::getNextConstraintBits(combinatorialPartition);
        uint64_t temp = bits & ((1ULL << sum) - 1);
        flags[temp] = true;
    }

    // Ensure no false flags i.e. we've seen every bitpattern
    for (auto const & b : flags) {
        REQUIRE(b.second == true);
    }
}