/**
 * @file: test_util.h
 *
 * @brief: Util functions for common functionality while testing
 * 
 * @author: Jake Tuero
 * Date: December 2019
 * Contact: tuero@ualberta.ca
 */


// Standard Libary/STL
#include <vector>

// Includes
#include "engine_types.h"


namespace testutil {

struct LevelSpriteInfo {
public:
    int levelNum;
    int levelWidth;
    int levelHeight;
    int numGemsNeeded;

    // gems
    std::vector<enginetype::GridCell> diamonds_bd;
    std::vector<enginetype::GridCell> emerald;  
    std::vector<enginetype::GridCell> diamonds_em;
    // rocks
    std::vector<enginetype::GridCell> rock_bd; 
    std::vector<enginetype::GridCell> rock_em; 
    // door
    std::vector<enginetype::GridCell> door;
    // keys
    std::vector<enginetype::GridCell> keys;
    // gates
    std::vector<enginetype::GridCell> gates;
    // dirt
    std::vector<enginetype::GridCell> dirts;
    // walls
    std::vector<enginetype::GridCell> walls;
    // misc
    std::vector<enginetype::GridCell> misc;
    // custom
    std::vector<enginetype::GridCell> custom;

    int numGems() {return diamonds_bd.size() + emerald.size() + diamonds_em.size();}

    int numRocks() {return rock_bd.size() + rock_em.size();}

    int numDoors() {return door.size();}

    int numKeys() {return keys.size();}

    int numGates() {return gates.size();}

    int numDirts() {return dirts.size();}
    int numWalls() {return walls.size();}

    int numMisc() {return misc.size();}

    int numCustom() {return custom.size();}

    int numSprites() {return numGems() + numRocks() + numDoors() + numKeys() + numGates() + numDirts() + numWalls() + numMisc() + numCustom();}

    std::vector<enginetype::GridCell> allGems() {
        std::vector<enginetype::GridCell> allSprites;
        allSprites.insert(allSprites.end(), diamonds_bd.begin(), diamonds_bd.end());
        allSprites.insert(allSprites.end(), emerald.begin(), emerald.end());
        allSprites.insert(allSprites.end(), diamonds_em.begin(), diamonds_em.end());
        return allSprites;
    }

    std::vector<enginetype::GridCell> allRocks() {
        std::vector<enginetype::GridCell> allSprites;
        allSprites.insert(allSprites.end(), rock_bd.begin(), rock_bd.end());
        allSprites.insert(allSprites.end(), rock_em.begin(), rock_em.end());
        return allSprites;
    }

    std::vector<enginetype::GridCell> allSprites() {
        std::vector<enginetype::GridCell> allSprites;
        allSprites.insert(allSprites.end(), diamonds_bd.begin(), diamonds_bd.end());
        allSprites.insert(allSprites.end(), emerald.begin(), emerald.end());
        allSprites.insert(allSprites.end(), diamonds_em.begin(), diamonds_em.end());
        allSprites.insert(allSprites.end(), rock_bd.begin(), rock_bd.end());
        allSprites.insert(allSprites.end(), rock_em.begin(), rock_em.end());
        allSprites.insert(allSprites.end(), door.begin(), door.end());
        allSprites.insert(allSprites.end(), keys.begin(), keys.end());
        allSprites.insert(allSprites.end(), gates.begin(), gates.end());
        allSprites.insert(allSprites.end(), dirts.begin(), dirts.end());
        allSprites.insert(allSprites.end(), walls.begin(), walls.end());
        allSprites.insert(allSprites.end(), misc.begin(), misc.end());
        allSprites.insert(allSprites.end(), custom.begin(), custom.end());
        return allSprites;
    }
};

static LevelSpriteInfo EMPTY_LEVEL = {
    1, // level num
    32, 64, // width and height
    0, // gems needed
    {}, {}, {}, // gems
    {}, {}, // rocks
    {},     // doors
    {}, // key
    {}, // gates
    {}, // dirt
    {}, // walls
    {}, // misc
    {} // custom
};
static LevelSpriteInfo FULL_LEVEL = {
    2, // level num
    32, 64, // width and height
    3, // gems needed
    {{2,0}, {0,9}, {4,9}, {9,9}}, {{4,0}}, {{6,0}},  // gems
    {{2,4}, {6,4}}, {{4,4}}, // rocks
    {{15,0}}, // doors
    {{0,13}, {3,13}, {6,13}, {9,13}},       // keys
    {{0,17}, {3,17}, {6,17}, {9,17}},      // gates
    {
    {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1}, {6,1}, {10,1},       // dirt
    {0,2}, {1,2}, {2,2}, {3,2}, {4,2}, {5,2}, {6,2}, {7,2}, {8,2}, {9,2}, {10,2},   //dirt
    {0,6}, {1,6}, {2,6}, {3,6}, {4,6}, {5,6}, {6,6}, {7,6}, {8,6}, {9,6}, {10,6},   //dirt
    {0,29}, {1,29}, {1,30}, {10,30},         // dirt
    {0,31}, {1,31}, {2,31}, {3,31}, {4,31}, {5,31}, {6,31}, {7,31}, {8,31}, {9,31}, {10,31},   //dirt
    },
    {
    {0,10}, {1,10}, {2,10}, {3,10}, {4,10}, {5,10}, {6,10}, {7,10}, {8,10}, {9,10}, {10,10},   //walls
    {0,14}, {1,14}, {2,14}, {3,14}, {4,14}, {5,14}, {6,14}, {7,14}, {8,14}, {9,14}, {10,14},   //walls
    {0,18}, {1,18}, {2,18}, {3,18}, {4,18}, {5,18}, {6,18}, {7,18}, {8,18}, {9,18}, {10,18},   //walls
    {0,22}, {1,22}, {2,22}, {3,22}, {4,22}, {5,22}, {6,22}, {7,22}, {8,22}, {9,22}, {10,22},   //walls
    {0,27}, {1,27}, {2,27}, {3,27}, {4,27}, {5,27}, {6,27}, {7,27}, {8,27}, {9,27}, {10,27},   //walls
    {0,8}, {1,8}, {1,9}, {10,9}, {0,25}, {1,25}, {1,26}, {10,26},         // walls
    },
    {{0,26}, {5,26}, {9,26}, {5,23}, // misc
    {0,30}, {5,30}, {9,30}},  // misc
    {{8,1}} // custom
};


/**
 * Load the test levelset and start the given level.
 * 
 * @param levelNum The level number to load.
 */
void loadTestLevelAndStart(int levelNum);

}