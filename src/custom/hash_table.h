#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <vector>
#include <limits>
#include <cstdint>

#include "util/rng.h"
#include "engine/engine_helper.h"

extern "C" {
    #include "../main.h"
    #include "../screens.h"
}

class StateHashTable {
private:
    static const int TT_SIZE = 15000000;    // TT size
    static const uint64_t MAX_HASH = UINT64_MAX;
    std::vector<uint64_t> ttTable; 

public:
    StateHashTable();

    /*
     * Check if the current engine state is in the hash table
     */
    bool in();

    /*
     * Add the current engine state to the hash table
     * Doesn't handle collisions
     */
    void add();
};



#endif