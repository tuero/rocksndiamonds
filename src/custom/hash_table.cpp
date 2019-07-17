

#include "hash_table.h"


StateHashTable::StateHashTable() {
    ttTable.clear();
    for (int i = 0; i < TT_SIZE; i++) {
        ttTable.push_back(0);
    }
}


/*
 * Check if the current engine state is in the hash table
 */
bool StateHashTable::in() { 
    uint64_t hash = enginehelper::stateToHash();
    return (ttTable[hash % TT_SIZE] != 0);
}


/*
 * Add the current engine state to the hash table
 * Doesn't handle collisions
 */
void StateHashTable::add() {
    ttTable[enginehelper::stateToHash() % TT_SIZE] = 1;
} 