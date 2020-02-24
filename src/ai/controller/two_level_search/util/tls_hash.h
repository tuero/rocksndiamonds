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
 * Create a hash for a given pair of indices for the master list of options availableOptions_.
 * Hash value is the string concatenation of the 2 indices.
 * 
 * @param indexCurr Index of the current item
 * @param indexPrev Index of the prev item
 * @param multiplier The multiplier being used during hashing
 * @return Hash representing the index of item prev -> index curr.
 */
inline uint64_t itemIndexPairToHash(std::size_t indexCurr, std::size_t indexPrev, std::size_t multiplier) {
    // indexCurr is the first option being executed.
    ++indexCurr; ++indexPrev;
    if (indexCurr == indexPrev) {return indexCurr;}

    return (indexCurr * multiplier) + indexPrev;
}


/**
 * Create hash for all pairs of items.
 * 
 * @param allItems Vector of items to create pairwise hashes.
 * @param multiplier The multiplier being used during hashing
 * @return A vector of all pairwise hashes.
 */
template <typename T>
std::vector<uint64_t> allItemsPairHashes(const std::vector<T> &allItems, std::size_t multiplier) {
    std::vector<uint64_t> itemPairHashes;
    for (std::size_t i = 0; i < allItems.size(); i++) {
        for (std::size_t j = 0; j < allItems.size(); j++) {
            itemPairHashes.push_back(itemIndexPairToHash(i, j, multiplier));
        }
    }
    return itemPairHashes;
}


/**
 * Create a hash for the given pair of items.
 * 
 * @param allItems Vector of all possible items.
 * @param multiplier The multiplier being used during hashing
 * @param currItem The 'to' item in the list of allItems
 * @param currItem The 'from' item in the list of allItems
 */
template <typename T>
uint64_t itemPairHash(const std::vector<T> &allItems, std::size_t multiplier, T currItem, T prevItem) {
    std::size_t indexCurr = std::distance(allItems.begin(), std::find(allItems.begin(), allItems.end(), currItem));
    std::size_t indexPrev = std::distance(allItems.begin(), std::find(allItems.begin(), allItems.end(), prevItem));
    return itemIndexPairToHash(indexCurr, indexPrev, multiplier);
}


/**
 * Create a hash for each pair of a given path of items. 
 * 
 * @param allItems Vector of all possible items.
 * @param path Subset of allItems which we wish to hash.
 * @return Vector of hashes for the given path.
 */
template <typename T>
std::vector<uint64_t> givenPathItemPairHashes(const std::vector<T> &allItems, uint64_t multiplier, const std::vector<T> &path) {
    std::vector<uint64_t> itemPairHashes;
    for (std::size_t i = 0; i < path.size(); i++) {
        std::size_t j = (i == 0) ? i : i - 1;
        itemPairHashes.push_back(itemPairHash(allItems, multiplier, path[i], path[j]));
    }
    return itemPairHashes;
}


/**
 * Convert the individual full path hash into the vector of hashes for each 
 * item pair in the path.
 * 
 * @param hash Hash of the path to decode
 * @param allItems Vector of all possible items.
 * @param multiplier Multiplier being used during hashing
 * @return Vector of pairwise hashes represented by the given full path hash
 */
template <typename T>
std::vector<uint64_t> pathHashToItemPairHash(uint64_t hash, const std::vector<T> &allItems, uint64_t multiplier) {
    std::size_t currIndex = (hash % multiplier) - 1;
    hash /= multiplier;

    std::vector<uint64_t> itemPairHashes;
    while (hash > 0) {
        std::size_t prevIndex = currIndex;
        currIndex = (hash % multiplier) - 1;
        itemPairHashes.insert(itemPairHashes.begin(), itemPairHash(allItems, multiplier, allItems[prevIndex], allItems[currIndex]));
        hash /= multiplier;
    }

    // First item in the path is paired with itself.
    itemPairHashes.insert(itemPairHashes.begin(), itemPairHash(allItems, multiplier, allItems[currIndex], allItems[currIndex]));
    return itemPairHashes;
}


/**
 * Get a pair of item indices represented by the given hash.
 * 
 * @param hash Hash of the path to decode
 * @param multiplier The multiplier being used during hashing
 * @return Array of indices represented by the given hash
 */
inline std::array<std::size_t, 2> hashToItemIndexPair(uint64_t hash, std::size_t multiplier) {
    if (hash < multiplier) {
        return {hash-1, hash-1};
    }
    return {(hash / multiplier) - 1, (hash % multiplier) - 1};
}


/**
 * Create a hash for a given path.
 * This hash represents all items in order in the path (can be more than 2)
 * 
 * @param allItems Vector of all possible items.
 * @param path Subset of allItems which we wish to hash.
 * @param multiplier The multiplier being used during hashing\
 * @return Hash for the given path
 */
template <typename T>
uint64_t itemPathToHash(const std::vector<T> &allItems, const std::vector<T> &path, std::size_t multiplier) {
    uint64_t hash = 0;
    for (auto const & item : path) {
        uint64_t index = std::distance(allItems.begin(), std::find(allItems.begin(), allItems.end(), item)) + 1;
        hash = (hash * multiplier) + index;
    }

    return hash;
}


/**
 * Create a hash for a given path.
 * This hash represents all items in order in the path (can be more than 2)
 * 
 * @param allItems Vector of all possible items.
 * @param path Subset of allItems which we wish to hash.
 * @param hash The hash representing the path
 * @param multiplier The multiplier being used during hashing
 * @return Vector of items representing the path by the given hash
 */
template <typename T>
std::vector<T> hashToItemPath(uint64_t hash, std::size_t multiplier, const std::vector<T> &allItems) {
    std::vector<T> items;
    while (hash > 0) {
        items.insert(items.begin(), allItems[(hash % multiplier) - 1]);
        hash /= multiplier;
    }

    return items;
}

} //namespace tlshash

#endif  //TLS_HASH