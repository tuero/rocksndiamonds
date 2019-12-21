/**
 * @file: engine_distance.cpp
 *
 * @brief: Implementation of distance functions from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include "engine_helper.h"

// Standard Libary/STL
#include <cmath>                // abs

// Includes
#include "engine_types.h"       // GridCell, Action
#include "rng.h"

// Game headers
extern "C" {
    #include "../../main.h"     // distances struct
}


namespace enginehelper {

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
uint64_t gridcellPathToHash(const std::deque<enginetype::GridCell> &path) {
    uint64_t hashValue = 0; 
    for (auto const & cell : path) {
        hashValue ^= zobristElement[cellToIndex(cell)][EL_EMPTY];
    }

    return hashValue;
}


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

} //namespace enginehelper