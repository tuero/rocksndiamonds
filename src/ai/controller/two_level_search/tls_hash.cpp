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
#include <deque>
#include <algorithm>
#include <iterator>

// Include
#include "logger.h"
#include "statistics.h"

using namespace enginehelper;

/**
 * Create hash for all pairs of options.
 */
std::vector<uint64_t> TwoLevelSearch::allOptionPairHashes() {
    std::vector<uint64_t> optionPairHashes;
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
uint64_t TwoLevelSearch::optionPairHash(BaseOption *currOption, BaseOption *prevOption) {
    int indexCurr = std::distance(availableOptions_.begin(), std::find(availableOptions_.begin(), availableOptions_.end(), currOption));
    int indexPrev = std::distance(availableOptions_.begin(), std::find(availableOptions_.begin(), availableOptions_.end(), prevOption));
    return optionIndexPairToHash(indexCurr, indexPrev);
}


/**
 * Create a hash for each pair of a given path of options. 
 */
std::vector<uint64_t> TwoLevelSearch::givenPathOptionPairHashes(const std::vector<BaseOption*> &path) {
    std::vector<uint64_t> optionPairHashes;
    for (int i = 0; i < (int)path.size(); i++) {
        int j = (i == 0) ? i : i - 1;
        optionPairHashes.push_back(optionPairHash(path[i], path[j]));
    }
    return optionPairHashes;
}


/**
 * Convert the inidividual full path hash into the vector of hashes for each 
 * option pair in the path.
 */
std::vector<uint64_t> TwoLevelSearch::pathHashToOptionPairHash(uint64_t hash) {
    std::vector<uint64_t> optionPairHashes;
    // hashToOptionPath(levinNode.hash)
    int currIndex = (hash % multiplier_) - 1;
    hash /= multiplier_;

    while (hash > 0) {
        int prevIndex = currIndex;
        currIndex = (hash % multiplier_) - 1;
        optionPairHashes.insert(optionPairHashes.begin(), optionPairHash(availableOptions_[prevIndex], availableOptions_[currIndex]));
        hash /= multiplier_;
    }
    optionPairHashes.insert(optionPairHashes.begin(), optionPairHash(availableOptions_[currIndex], availableOptions_[currIndex]));
    return optionPairHashes;
}



/**
 * Create a hash for a given pair of indices for the master list of options availableOptions_.
 * Hash value is the string concatenation of the 2 indices.
 */
uint64_t TwoLevelSearch::optionIndexPairToHash(int indexCurr, int indexPrev) {
    // indexCurr should always be a valid index in availableOptions_.
    if (indexCurr < 0 || indexCurr >= (int)availableOptions_.size()) {
        PLOGE_(logger::FileLogger) << "Invalid current index.";
    }

    // indexCurr is the first option being executed.
    ++indexCurr; ++indexPrev;
    if (indexCurr == indexPrev) {return (uint64_t)indexCurr;}

    return (indexCurr * multiplier_) + indexPrev;
}


/**
 * Get a pair of options represented by the given hash.
 */
TwoLevelSearch::OptionIndexPair TwoLevelSearch::hashToOptionIndexPair(uint64_t hash) {
    if (hash < multiplier_) {
        return {hash-1, hash-1};
    }

    return {(hash / multiplier_) - 1, (hash % multiplier_) - 1};
}


/**
 * Create a hash for a given path.
 * This hash represents all options in order in the path (can be more than 2)
 */
uint64_t TwoLevelSearch::optionPathToHash(const std::vector<BaseOption*> &path) {
    uint64_t hash = 0;
    for (auto const & option : path) {
        // Get option index
        auto iter = std::find(availableOptions_.begin(), availableOptions_.end(), option);
        uint64_t index = std::distance(availableOptions_.begin(), iter) + 1;
        hash = (hash * multiplier_) + index;
    }

    return hash;
}


std::vector<BaseOption*> TwoLevelSearch::hashToOptionPath(uint64_t hash) {
    std::vector<BaseOption*> options;
    while (hash > 0) {
        int index = (hash % multiplier_) - 1;
        options.insert(options.begin(), availableOptions_[index]);
        hash /= multiplier_;
    }

    return options;
}


void TwoLevelSearch::incrementPathTimesVisited(const std::vector<BaseOption*> &path) {
    // std::deque<BaseOption*> pathVisited(pathContainer.begin(), pathContainer.end());
    // // Hash each partial path after the next step is taken
    // while (!pathVisited.empty()) {
    //     uint64_t hash = optionPathToHash(pathVisited);
    //     ++hashPathTimesVisited[hash];
    //     pathVisited.pop_front();
    // }

    // Statistics logging
    ++(statistics::pathCounts[levelinfo::getLevelNumber()][currentHighLevelPathHash_]);
    statistics::solutionPathCounts[levelinfo::getLevelNumber()][0] = currentHighLevelPathHash_;
    statistics::solutionPathCounts[levelinfo::getLevelNumber()][1] = ++hashPathTimesVisited[currentHighLevelPathHash_];
}

