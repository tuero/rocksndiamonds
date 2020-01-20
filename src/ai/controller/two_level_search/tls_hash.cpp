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
template<typename T>
std::vector<uint64_t> TwoLevelSearch::givenPathOptionPairHashes(const T &pathContainer) {
    std::vector<uint64_t> optionPairHashes;
    for (int i = 0; i < (int)pathContainer.size(); i++) {
        int j = (i == 0) ? i : i - 1;
        optionPairHashes.push_back(optionPairHash(pathContainer[i], pathContainer[j]));
    }
    return optionPairHashes;
}
template std::vector<uint64_t> TwoLevelSearch::givenPathOptionPairHashes<std::vector<BaseOption*>> (const std::vector<BaseOption*>&);
template std::vector<uint64_t> TwoLevelSearch::givenPathOptionPairHashes<std::deque<BaseOption*>> (const std::deque<BaseOption*>&);



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
    if (indexCurr == indexPrev) {return (uint64_t)indexCurr;}

    return ((indexCurr + 1) * multiplier_) + indexPrev;
}


/**
 * Get a pair of options represented by the given hash.
 */
TwoLevelSearch::OptionIndexPair TwoLevelSearch::hashToOptionIndexPair(uint64_t hash) {
    if (hash < (uint64_t)availableOptions_.size()) {
        return {hash, hash};
    }

    return {(hash / multiplier_) - 1, hash % multiplier_};
}


/**
 * Create a hash for a given path.
 * This hash represents all options in order in the path (can be more than 2)
 */
template<typename T>
uint64_t TwoLevelSearch::optionPathToHash(const T &pathContainer) {
    uint64_t hash = 0;
    for (auto const & option : pathContainer) {
        // Get option index
        auto iter = std::find(availableOptions_.begin(), availableOptions_.end(), option);
        uint64_t index = std::distance(availableOptions_.begin(), iter) + 1;
        hash = (hash * multiplier_) + index;
    }

    return hash;
}
template uint64_t TwoLevelSearch::optionPathToHash<std::vector<BaseOption*>> (const std::vector<BaseOption*>&);
template uint64_t TwoLevelSearch::optionPathToHash<std::deque<BaseOption*>> (const std::deque<BaseOption*>&);


std::vector<BaseOption*> TwoLevelSearch::hashToOptionPath(uint64_t hash) {
    std::vector<BaseOption*> options;
    while (hash > 0) {
        int index = (hash % multiplier_) - 1;
        options.insert(options.begin(), availableOptions_[index]);
        hash /= multiplier_;
    }

    return options;
}


template<typename T>
void TwoLevelSearch::incrementPathTimesVisited(const T &pathContainer) {
    std::deque<BaseOption*> pathVisited(pathContainer.begin(), pathContainer.end());
    // Hash each partial path after the next step is taken
    while (!pathVisited.empty()) {
        uint64_t hash = optionPathToHash(pathVisited);
        ++hashPathTimesVisited[hash];
        pathVisited.pop_front();
    }
}
template void TwoLevelSearch::incrementPathTimesVisited<std::vector<BaseOption*>> (const std::vector<BaseOption*>&);
template void TwoLevelSearch::incrementPathTimesVisited<std::deque<BaseOption*>> (const std::deque<BaseOption*>&);


template<typename T>
int TwoLevelSearch::getPathTimesVisited(const T &pathContainer) {
    uint64_t hash = optionPathToHash(pathContainer);
    auto iterator = hashPathTimesVisited.find(hash);
    return (iterator == hashPathTimesVisited.end()) ? 1 : iterator->second + 1;
}
template int TwoLevelSearch::getPathTimesVisited<std::vector<BaseOption*>> (const std::vector<BaseOption*>&);
template int TwoLevelSearch::getPathTimesVisited<std::deque<BaseOption*>> (const std::deque<BaseOption*>&);
