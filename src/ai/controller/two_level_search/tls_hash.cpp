/**
 * @file: tls_hash.cpp
 *
 * @brief: Path hashing functions for Masters thesis controller.
 * 
 * @author: Jake Tuero
 * Date: January 2020
 * Contact: tuero@ualberta.ca
 */


#include "two_level_search.h"

// Standard Libary/STL
#include <vector>
#include <algorithm>
#include <iterator>

// Include
#include "logger.h"


/**
 * Create hash for all pairs of options.
 */
std::vector<int> TwoLevelSearch::allOptionPairHashes() {
    std::vector<int> optionPairHashes;
    for (int i = 0; i < (int)availableOptions_.size(); i++) {
        for (int j = 0; j < (int)availableOptions_.size(); j++) {
            optionPairHashes.push_back(optionIndexPairToHash(i, j));
        }
    }
    return optionPairHashes;
}


/**
 * Create a hash for the given pair of options. 
 */
int TwoLevelSearch::optionPairHash(BaseOption *currOption, BaseOption *prevOption) {
    int indexCurr = std::distance(availableOptions_.begin(), std::find(availableOptions_.begin(), availableOptions_.end(), currOption));
    int indexPrev = std::distance(availableOptions_.begin(), std::find(availableOptions_.begin(), availableOptions_.end(), prevOption));
    return optionIndexPairToHash(indexCurr, indexPrev);
}


/**
 * Create a hash for each pair of a given path of options. 
 */
std::vector<int> TwoLevelSearch::givenPathOptionPairHashes(std::vector<BaseOption*> path) {
    std::vector<int> optionPairHashes;
    for (int i = 0; i < (int) path.size(); i++) {
        int j = (i == 0) ? i : i - 1;
        optionPairHashes.push_back(optionPairHash(path[i], path[j]));
    }
    return optionPairHashes;
}


/**
 * Create a hash for a given pair of indices for the master list of options availableOptions_.
 * Hash value is the string concatenation of the 2 indices.
 */
int TwoLevelSearch::optionIndexPairToHash(int indexCurr, int indexPrev) {
    // indexCurr should always be a valid index in availableOptions_.
    if (indexCurr < 0 || indexCurr >= (int)availableOptions_.size()) {
        PLOGE_(logger::FileLogger) << "Invalid current index.";
    }

    // indexCurr is the first option being executed.
    if (indexCurr == indexPrev) {return (uint64_t)indexCurr;}

    int multiplier = 10;
    while (multiplier < (int)availableOptions_.size()) {
        multiplier *= 10;
    }

    return ((indexCurr + 1) * multiplier) + indexPrev;
}


/**
 * Get a pair of options represented by the given hash.
 */
TwoLevelSearch::OptionIndexPair TwoLevelSearch::hashToOptionIndexPair(int hash) {
    if (hash < (int)availableOptions_.size()) {
        return {hash, hash};
    }

    int multiplier = 10;
    while (multiplier < (int)availableOptions_.size()) {
        multiplier *= 10;
    }

    return {(hash / multiplier) - 1, hash % multiplier};
}


/**
 * Create a hash for a given path.
 * This hash represents all options in order in the path (can be more than 2)
 */
uint64_t TwoLevelSearch::optionPathToHash(std::vector<BaseOption*> path) {
    uint64_t hash = 0;
    uint64_t multiplier = 10;
    while (multiplier < (uint64_t)availableOptions_.size()) {
        multiplier *= 10;
    }
    for (auto const & option : path) {
        // Get option index
        auto iter = std::find(availableOptions_.begin(), availableOptions_.end(), option);
        uint64_t index = std::distance(availableOptions_.begin(), iter);
        hash = (hash * multiplier) + index;
    }

    return hash;
}
