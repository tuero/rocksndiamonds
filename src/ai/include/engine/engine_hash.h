/**
 * @file: engine_hash.cpp
 *
 * @brief: Interface for controllers to access hashing function for the engine state.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


// Standard Library/STL
#include <deque>

// Include
#include "engine_types.h"


namespace enginehelper {
namespace enginehash {

    /**
     * Initialize Zorbrist tables, used to hash game board states
     */
    void initZorbristTables();

    /**
     * Get the hash representation of a vector grid cell path.
     */
    template<typename Iter>
    uint64_t gridcellPathToHash(Iter iter, Iter end);

    /**
     * Get the hash representation of the current state in the engine
     */
    uint64_t stateToHash();

} // namespace enginehash
} // namespace enginehelper