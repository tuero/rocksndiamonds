

#include "hash_table.h"


StateHashTable::StateHashTable() {
    initZobristTable();
    ttTable.clear();
    for (int i = 0; i < TT_SIZE; i++) {
        ttTable.push_back(0);
    }
}


/*
 * Initialize pseudo-randomly the Zobrist table for the initial board state
 */
void StateHashTable::initZobristTable() {
    // Initialize each starting configuration with pseudo-random number
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, MAX_HASH);

    for (int i = 0; i < MAX_LEV_FIELDX*MAX_LEV_FIELDY; i++) {
        // K is item type
        for (int k = 0; k < MAX_NUM_ELEMENTS; k++) {
            zobristElement[i][k] = dis(gen);
        }
        for (int k = 0; k < maxDir; k++) {
            zobristDir[i][k] = dis(gen);
        }
    }
}


/*
 * Initialize the hash value for the starting board state
 */
uint64_t StateHashTable::calcHash() {
    int px = stored_player[0].jx;
    int py = stored_player[0].jy;
    int pMov = stored_player[0].MovDir;
    uint64_t hashValue = 0; 

    // Set initial hash
    for (int x = 0; x < level.fieldx; x++) {
        for (int y = 0; y < level.fieldy; y++) {
            hashValue ^= zobristElement[y*level.fieldx + x][Feld[x][y]];
            hashValue ^= zobristDir[y*level.fieldx + x][MovDir[x][y]];
        }
    }

    hashValue ^= zobristElement[py*level.fieldx + px][80];
    hashValue ^= zobristDir[py*level.fieldx + px][pMov];

    return hashValue;
}


bool StateHashTable::in() { 
    uint64_t hash = calcHash();
    return (ttTable[hash % TT_SIZE] != 0);
}


void StateHashTable::add() {
    ttTable[calcHash() % TT_SIZE] = 1;
} 