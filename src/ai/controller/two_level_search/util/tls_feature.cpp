/**
 * @file: tls_feature.cpp
 *
 * @brief: Construct the features for learning
 * 
 * @author: Jake Tuero
 * Date: February 2020
 * Contact: tuero@ualberta.ca
 */


// Standard Libary/STL
#include <cstdint>              // fixed-width datatypes
#include <vector>
#include <algorithm>            // std::find, distance

// Includes
#include "base_option.h"
#include "tls_hash.h"

#ifdef RUN_TESTS
#include <catch2/catch.hpp>
#include <iostream>
#include "option_types.h"
#include "option_factory.h"
#include "engine_helper.h"
#include "engine_types.h"
#include "../../../tests/test_util.h"

using namespace enginehelper;
#endif

/**
 * Get the vector of HLA ordering flags.
 */
template <typename T>
std::vector<double> _orderings(uint64_t path_hash, std::size_t multiplier, std::vector<T> &allItems) {
    std::vector<T> path = tlshash::hashToItemPath(path_hash, multiplier, allItems);

    std::size_t num_pairs = allItems.size() * (allItems.size() - 1);
    std::vector<double> feature_orderings(num_pairs, 0);
    for (std::size_t i = 0; i < path.size(); ++i) {
        for (std::size_t j = i+1; j < path.size(); ++j) {
            std::size_t index_prev = std::distance(allItems.begin(), std::find(allItems.begin(), allItems.end(), path[i]));
            std::size_t index_curr = std::distance(allItems.begin(), std::find(allItems.begin(), allItems.end(), path[j]));
            std::size_t index = ((index_prev * allItems.size()) + index_curr - index_prev) - ((index_prev < index_curr) ? 1 : 0);
            feature_orderings[index] = 1;
        }
    }
    
    return feature_orderings;
}

double _avg_constraints(int numConstraints, int pathLength) {
    return (double)numConstraints / pathLength;
}

double _total_constraints(int numConstraints) {
    return (double)numConstraints;
}

double _avg_bitwalls(uint64_t path_hash, std::size_t multiplier, std::vector<BaseOption*> &allItems) {
    std::vector<BaseOption*> path = tlshash::hashToItemPath(path_hash, multiplier, allItems);

    // Walk along the path and count the number of bitwalls
}

double _total_bitwalls(uint64_t path_hash, std::size_t multiplier, std::vector<BaseOption*> &allItems) {
    std::vector<BaseOption*> path = tlshash::hashToItemPath(path_hash, multiplier, allItems);

    // Walk along the path and count the number of bitwalls
}



std::vector<double> getFeatureFector() {
    std::vector<double> features;

    // Bool flags for pariwise ordering

    return features;
}



// ------------------------------ Tests ---------------------------------
#ifdef RUN_TESTS
template <typename T>
uint64_t _getMultiplier(std::vector<T> &allItems) {
    uint64_t multiplier = 10;
    while (multiplier < (uint64_t)allItems.size()) {
        multiplier *= 10;
    }
    return multiplier;
}

TEST_CASE("TLS feature orderings", "[tls_features]") {
    std::vector<int> allItems{1, 2, 3, 4, 5};
    uint64_t multiplier = _getMultiplier(allItems);

    SECTION("Path 1") {
        std::vector<int> partial_path{1, 2, 3, 4, 5};
        uint64_t path_hash = tlshash::itemPathToHash(allItems, partial_path, multiplier);
        std::vector<double> flags = _orderings(path_hash, multiplier, allItems);
        std::vector<double> set_flags{1, 1, 1, 1,           // 1, i
                                      0, 1, 1, 1,           // 2, i
                                      0, 0, 1, 1,           // 3, i
                                      0, 0, 0, 1,           // 4, i
                                      0, 0, 0, 0            // 5, i
                                    };
        REQUIRE(flags == set_flags);
    }
    SECTION("Path 2") {
        std::vector<int> partial_path{5, 4, 3, 2, 1};
        uint64_t path_hash = tlshash::itemPathToHash(allItems, partial_path, multiplier);
        std::vector<double> flags = _orderings(path_hash, multiplier, allItems);
        std::vector<double> set_flags{0, 0, 0, 0,           // 1, i
                                      1, 0, 0, 0,           // 2, i
                                      1, 1, 0, 0,           // 3, i
                                      1, 1, 1, 0,           // 4, i
                                      1, 1, 1, 1            // 5, i
                                    };
        REQUIRE(flags == set_flags);
    }
    SECTION("Path 3") {
        std::vector<int> partial_path{3, 1, 5};
        uint64_t path_hash = tlshash::itemPathToHash(allItems, partial_path, multiplier);
        std::vector<double> flags = _orderings(path_hash, multiplier, allItems);
        std::vector<double> set_flags{0, 0, 0, 1,           // 1, i
                                      0, 0, 0, 0,           // 2, i
                                      1, 0, 0, 1,           // 3, i
                                      0, 0, 0, 0,           // 4, i
                                      0, 0, 0, 0            // 5, i
                                    };
        REQUIRE(flags == set_flags);
    }
}

TEST_CASE("TLS feature bits", "[tls_features]") {

    SECTION("Path 1") {
        OptionFactory factory;
        testutil::loadTestLevelAndStart(3);
        std::vector<BaseOption*> allOptions = factory.createOptions(OptionFactoryType::TWO_LEVEL_SEARCH);
        uint64_t multiplier = _getMultiplier(allOptions);

        for (auto const & option : allOptions) {
            std::cout << option->toString() << std::endl;
        }

        std::vector<BaseOption*> partial_path{allOptions[6], allOptions[3]};
        uint64_t path_hash = tlshash::itemPathToHash(allOptions, partial_path, multiplier);
        double totalBits = _total_bitwalls(path_hash, multiplier, allOptions);
    }
    
    
}

#endif

