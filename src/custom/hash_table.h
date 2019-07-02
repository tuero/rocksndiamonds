#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <limits>
#include <cstdint>

extern "C" {
    #include "../main.h"
    #include "../screens.h"
}

class StateHashTable {
private:
    static const int TT_SIZE = 15000000;    // TT size
    static const uint64_t MAX_HASH = UINT64_MAX;
    static const int maxDir = 32;
    uint64_t zobristElement[MAX_LEV_FIELDX * MAX_LEV_FIELDY][MAX_NUM_ELEMENTS];  // Zobrist table
    uint64_t zobristDir[MAX_LEV_FIELDX * MAX_LEV_FIELDY][maxDir];      // Zobrist table
    std::vector<uint64_t> ttTable; 

public:
    StateHashTable();
    void initZobristTable();
    void initHashValue();
    uint64_t calcHash();
    bool in();
    void add();
};



#endif