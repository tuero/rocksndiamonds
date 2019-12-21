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
    // custom
    std::vector<enginetype::GridCell> custom;

    int numGems() {return diamonds_bd.size() + emerald.size() + diamonds_em.size();}

    int numRocks() {return rock_bd.size() + rock_em.size();}

    int numDoors() {return door.size();}

    int numCustom() {return custom.size();}

    int numSprites() {return numGems() + numRocks() + numDoors() + numCustom();}

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
        allSprites.insert(allSprites.end(), custom.begin(), custom.end());
        return allSprites;
    }
};

static LevelSpriteInfo EMPTY_LEVEL = {1, 32, 64, 0, {}, {}, {}, {}, {}, {}, {}};
static LevelSpriteInfo FULL_LEVEL = {2, 32, 64, 3, {{2,0}}, {{4,0}}, {{6,0}}, {{2,4}, {6,4}}, {{4,4}}, {{15,0}}, {{8,1}}};


/**
 * Load the test levelset and start the given level.
 * 
 * @param levelNum The level number to load.
 */
void loadTestLevelAndStart(int levelNum);

}