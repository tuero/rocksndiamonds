/**
 * @file: tls_hash_test.cpp
 *
 * @brief: Test functions for hashing helper functions for controller TLS
 * @author: Jake Tuero
 * Date: February 2020
 * Contact: tuero@ualberta.ca
 */


// Testing framework
#include <catch2/catch.hpp>

// Standard Libary/STL
#include <vector>
#include <array>

// Includes
#include "../../controller/two_level_search/util/tls_hash.h"


uint64_t getMultiplier(std::vector<int> &allItems) {
    uint64_t multiplier = 10;
    while (multiplier < (uint64_t)allItems.size()) {
        multiplier *= 10;
    }
    return multiplier;
}



/**
 * Test the itemIndexPairToHash function 
 */
TEST_CASE("TLS item index pair to hash", "[tls_hash]") {
    std::vector<int> allItems{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    uint64_t multiplier = getMultiplier(allItems);

    size_t currIndex = 0;
    size_t prevIndex = 4;

    SECTION("Same item hash") {
        REQUIRE(tlshash::itemIndexPairToHash(currIndex, currIndex, multiplier) == 1);
    }
    SECTION("Different item hash") {
        REQUIRE(tlshash::itemIndexPairToHash(currIndex, prevIndex, multiplier) == 105);
    }
}


/**
 * Test the allItemsPairHashes function 
 */
TEST_CASE("TLS all items pair hashes", "[tls_hash]") {
    std::vector<int> allItems{1, 2, 3, 4, 5};
    uint64_t multiplier = getMultiplier(allItems);

    std::vector<uint64_t> itemPairHashes = tlshash::allItemsPairHashes(allItems, multiplier);
    std::vector<uint64_t> correctPairHashes{1, 12, 13, 14, 15, 21, 2, 23, 24, 25, 31, 32, 3, 34, 35, 41, 42, 43, 4, 45, 51, 52, 53, 54, 5};

    REQUIRE(itemPairHashes == correctPairHashes);
}


/**
 * Test the itemPairHash function 
 */
TEST_CASE("TLS item pair hashes", "[tls_hash]") {
    std::vector<int> allItems{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    uint64_t multiplier = getMultiplier(allItems);

    SECTION("Same item hash") {
        REQUIRE(tlshash::itemPairHash(allItems, multiplier, 11, 11) == 11);
    }
    SECTION("Different item hash") {
        REQUIRE(tlshash::itemPairHash(allItems, multiplier, 11, 1) == 11*multiplier+1);
    }
}


/**
 * Test the givenPathItemPairHashes function 
 */
TEST_CASE("TLS given path item pair hashes", "[tls_hash]") {
    std::vector<int> allItems{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    std::vector<int> subsetItems{1, 6, 10, 11};
    uint64_t multiplier = getMultiplier(allItems);

    SECTION("No path") {
        REQUIRE(tlshash::givenPathItemPairHashes(allItems, multiplier, {}) == (std::vector<uint64_t>){});
    }
    SECTION("Different item hash") {
        REQUIRE(tlshash::givenPathItemPairHashes(allItems, multiplier, subsetItems) == (std::vector<uint64_t>){1, 601, 1006, 1110});
    }
}


/**
 * Test the hashToItemIndexPair function 
 */
TEST_CASE("TLS hash to item index pair", "[tls_hash]") {
    std::vector<int> allItems{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    uint64_t multiplier = getMultiplier(allItems);

    std::vector<std::array<std::size_t, 2>> pairs{{0, 0}, {5, 0}, {9, 5}, {10, 9}};
    std::vector<uint64_t> hashes{1, 601, 1006, 1110};

    for (std::size_t i = 0; i < hashes.size(); ++i) {
        REQUIRE(tlshash::hashToItemIndexPair(hashes[i], multiplier) == pairs[i]);
    }
}



/**
 * Test the itemPathToHash and hashToItemPath function s
 */
TEST_CASE("TLS hashing and pathing", "[tls_hash]") {
    // std::vector<int> allItems{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    std::vector<int> allItems{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint64_t multiplier = getMultiplier(allItems);

    // std::vector<int> subsetItems{1, 6, 10, 11};
    std::vector<int> subsetItems{6, 1, 2, 0, 7, 4, 3, 7, 8, 10, 5, 9, 5};
    uint64_t hash = 1061011;

    REQUIRE(tlshash::itemPathToHash(allItems, subsetItems, multiplier) == hash);

    SECTION("Path to hash") {
        REQUIRE(tlshash::itemPathToHash(allItems, subsetItems, multiplier) == hash);
    }
    SECTION("Hash to path") {
        REQUIRE(tlshash::hashToItemPath(hash, multiplier, allItems) == subsetItems);
    }
    SECTION("Path to hash to path") {
        REQUIRE(tlshash::hashToItemPath(tlshash::itemPathToHash(allItems, subsetItems, multiplier), multiplier, allItems) == subsetItems);
    }
    SECTION("Hash to path to hash") {
        REQUIRE(tlshash::itemPathToHash(allItems, tlshash::hashToItemPath(hash, multiplier, allItems), multiplier) == hash);
    }
}