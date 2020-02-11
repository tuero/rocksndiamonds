/**
 * @file: engine_hash.cpp
 *
 * @brief: Implementation of hashing function for the engine state.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include "engine_helper.h"

// Standard Libary/STL
#include <vector>
#include <deque>
#include <cmath>                // abs

// Includes
#include "engine_types.h"       // GridCell, Action
#include "rng.h"

// Game headers
extern "C" {
    #include "../../main.h"     // distances struct
}


namespace enginehelper {
namespace enginehash {

const uint64_t MAX_HASH = UINT64_MAX;
const int MAX_DIR = 32;

// Hashing data structures
uint64_t zobristElement[MAX_LEV_FIELDX * MAX_LEV_FIELDY][MAX_NUM_ELEMENTS];


/**
 * Initialize Zorbrist tables, used to hash game board states
 */
void initZorbristTables() {
    for (int i = 0; i < MAX_LEV_FIELDX*MAX_LEV_FIELDY; i++) {
        // K is item type
        for (int k = 0; k < MAX_NUM_ELEMENTS; k++) {
            zobristElement[i][k] = RNG::getRandomNumber();
        }
    }
}


/**
 * Get the hash representation of a vector grid cell path.
 */
template<typename Iter>
uint64_t gridcellPathToHash(Iter iter, Iter end) {
    uint64_t hashValue = 0; 
    for (; iter != end; ++iter) {
        hashValue ^= zobristElement[gridinfo::cellToIndex(*iter)][EL_EMPTY];
    }

    return hashValue;
}
template uint64_t gridcellPathToHash(std::vector<enginetype::GridCell>::iterator, std::vector<enginetype::GridCell>::iterator);
template uint64_t gridcellPathToHash(std::deque<enginetype::GridCell>::iterator, std::deque<enginetype::GridCell>::iterator);


/**
 * Get the hash representation of the current state in the engine
 */
uint64_t stateToHash() {
    int px = stored_player[0].jx;
    int py = stored_player[0].jy;
    uint64_t hashValue = 0; 

    // Set initial hash
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            hashValue ^= zobristElement[y*level.fieldx + x][Feld[x][y]];
        }
    }

    hashValue ^= zobristElement[py*level.fieldx + px][80];

    return hashValue;
}

} //namespace enginehash
} //namespace enginehelper