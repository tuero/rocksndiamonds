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
    // misc
    std::vector<enginetype::GridCell> misc;
    // custom
    std::vector<enginetype::GridCell> custom;

    int numGems() {return diamonds_bd.size() + emerald.size() + diamonds_em.size();}

    int numRocks() {return rock_bd.size() + rock_em.size();}

    int numDoors() {return door.size();}

    int numMisc() {return misc.size();}

    int numCustom() {return custom.size();}

    int numSprites() {return numGems() + numRocks() + numDoors() + numMisc() + numCustom();}

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
    {}, // doors
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
    {{0,13}, {3,13}, {6,13}, {9,13}, 
    {0,17}, {3,17}, {6,17}, {9,17},
    {0,26}, {5,26}, {9,26}, {5,23},
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