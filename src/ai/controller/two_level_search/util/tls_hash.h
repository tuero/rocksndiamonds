/**
 * @file: tls_hash.h
 *
 * @brief: Path hashing functions for Masters thesis controller.
 * 
 * @author: Jake Tuero
 * Date: January 2020
 * Contact: tuero@ualberta.ca
 */

#ifndef TLS_HASH
#define TLS_HASH

// Standard Libary/STL
#include <vector>
#include <array>
#include <algorithm>
#include <iterator>

// Include
#include "logger.h"


namespace tlshash {


/**
 * Taken from boost::hash_combine
 */
inline void _hash_combine(size_t & seed, uint64_t h) {
    seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2);    
}


/**
 * Create hash for the given path
 * 
 * @param allItems Vector of items to create pairwise hashes.
 * @param path Subset of allItems which we wish to hash
 * @return A single hash representing the path.
 */
template <typename T>
uint64_t hashPath(const std::vector<T> &allItems, const std::vector<T> &path) {
    uint64_t hash = 0;
    for (auto const & item : path) {
        uint64_t index = std::distance(allItems.begin(), std::find(allItems.begin(), allItems.end(), item));
        _hash_combine(hash, index);
    }

    return hash;
}


/**
 * Create hash for all pairs of items.
 * 
 * @param allItems Vector of items to create pairwise hashes.
 * @return A vector of all pairwise hashes.
 */
template <typename T>
std::vector<uint64_t> allItemsPairHashes(const std::vector<T> &allItems) {
    std::vector<uint64_t> itemPairHashes;
    for (std::size_t i = 0; i < allItems.size(); i++) {
        for (std::size_t j = 0; j < allItems.size(); j++) {
            itemPairHashes.push_back(hashPath(allItems, (i == j) ? std::vector<T>{allItems[i]} : std::vector<T>{allItems[i], allItems[j]}));
        }
    }
    return itemPairHashes;
}


/**
 * Create a hash for each pair of a given path of items. 
 * 
 * @param allItems Vector of all possible items.
 * @param path Subset of allItems which we wish to hash.
 * @return Vector of hashes for the given path.
 */
template <typename T>
std::vector<uint64_t> pathToPairHashes(const std::vector<T> &allItems, const std::vector<T> &path) {
    std::vector<uint64_t> itemPairHashes;
    for (std::size_t j = 0; j < path.size(); ++j) {
        std::size_t i = (j == 0) ? j : j - 1;
        itemPairHashes.push_back(hashPath(allItems, (i == j) ? std::vector<T>{path[i]} : std::vector<T>{path[i], path[j]}));
    }
    return itemPairHashes;
}

} //namespace tlshash

#endif  //TLS_HASH