/**
 * @file: engine_grid_info_test.cpp
 *
 * @brief: Test functions for the grid information section of the engine helper
 * 
 * @author: Jake Tuero
 * Date: December 2019
 * Contact: tuero@ualberta.ca
 */

// Testing framework
#include <catch2/catch.hpp>

// Standard Libary/STL
#include <algorithm>        // std::max, std::find
#include <vector>

// Includes
#include "engine_types.h"
#include "engine_helper.h"

// Testing util
#include "test_util.h"

using namespace enginehelper;


/**
 * Test for cells in bounds.
 */
TEST_CASE("Check In Bounds", "[gridcell_information]") {
    int levelNum = testutil::EMPTY_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    int levelWidth = levelinfo::getLevelWidth();
    int levelHeight = levelinfo::getLevelHeight();

    // Out of bounds on either side(s)
    REQUIRE(!gridinfo::inBounds({-1, -1}));
    REQUIRE(!gridinfo::inBounds({levelWidth, -1}));
    REQUIRE(!gridinfo::inBounds({-1, levelHeight}));
    REQUIRE(!gridinfo::inBounds({levelWidth, levelHeight}));

    // Check in bounds
    REQUIRE(gridinfo::inBounds({0, 0}));
    REQUIRE(gridinfo::inBounds({levelWidth-1, 0}));
    REQUIRE(gridinfo::inBounds({0, levelHeight-1}));
    REQUIRE(gridinfo::inBounds({levelWidth-1, levelHeight-1}));
}


/**
 * Test for cells to index
 */
TEST_CASE("Check Cell To Index", "[gridcell_information]") {
    int levelNum = testutil::EMPTY_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    int levelWidth = levelinfo::getLevelWidth();
    int levelHeight = levelinfo::getLevelHeight();

    // Cell not in bounds
    REQUIRE(gridinfo::cellToIndex({-1, -1}) == -1);
    REQUIRE(gridinfo::cellToIndex({levelWidth, -1}) == -1);
    REQUIRE(gridinfo::cellToIndex({-1, levelHeight}) == -1);
    REQUIRE(gridinfo::cellToIndex({levelWidth, levelHeight}) == -1);

    // Cell in bounds
    REQUIRE(gridinfo::cellToIndex({0, 0}) == 0);
    REQUIRE(gridinfo::cellToIndex({levelWidth-1, 0}) == levelWidth-1);
    REQUIRE(gridinfo::cellToIndex({0, levelHeight-1}) == (levelHeight-1)*(levelWidth));
    REQUIRE(gridinfo::cellToIndex({levelWidth-1, levelHeight-1}) == (levelWidth*levelHeight) - 1);
}


/**
 * Test for cells to index
 */
TEST_CASE("Check Index To Cell", "[gridcell_information]") {
    int levelNum = testutil::EMPTY_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    int levelWidth = levelinfo::getLevelWidth();
    int levelHeight = levelinfo::getLevelHeight();

    // Cell not in bounds
    REQUIRE(gridinfo::indexToCell(-1) == (enginetype::GridCell){-1, -1});
    REQUIRE(gridinfo::indexToCell(levelWidth*levelHeight) == (enginetype::GridCell){-1, -1});

    // Cell in bounds
    REQUIRE(gridinfo::indexToCell(0) == (enginetype::GridCell){0, 0});
    REQUIRE(gridinfo::indexToCell(levelWidth-1) == (enginetype::GridCell){levelWidth-1, 0});
    REQUIRE(gridinfo::indexToCell((levelHeight-1)*levelWidth) == (enginetype::GridCell){0, levelHeight-1});
    REQUIRE(gridinfo::indexToCell((levelWidth*levelHeight) - 1) == (enginetype::GridCell){levelWidth-1, levelHeight-1});
}


/**
 * Test for sprite ID initialization and retrieving them.
 */
TEST_CASE("Check the init sprite IDs", "[gridcell_information]") {
    SECTION("Empty level") {
        int levelNum = testutil::EMPTY_LEVEL.levelNum;
        testutil::loadTestLevelAndStart(levelNum);

        int levelWidth = levelinfo::getLevelWidth();
        int levelHeight = levelinfo::getLevelHeight();

        // Initialize the sprite IDs
        gridinfo::initSpriteIDs();

        // Ensure all spriteIDs are -1, as there are no valid sprites on the empty map
        for (int x = 0; x < levelWidth; x++) {
            for (int y = 0; y < levelHeight; y++) {
                REQUIRE(gridinfo::getSpriteID({x,y}) == -1);
            }
        }
        // Out of bounds
        REQUIRE(gridinfo::getSpriteID({-1,-1}) == -1);
        REQUIRE(gridinfo::getSpriteID({levelWidth, levelHeight}) == -1);
    }
    SECTION("Full level") {
        int levelNum = testutil::FULL_LEVEL.levelNum;
        testutil::loadTestLevelAndStart(levelNum);

        int levelWidth = levelinfo::getLevelWidth();
        int levelHeight = levelinfo::getLevelHeight();

        // Initialize the sprite IDs
        gridinfo::initSpriteIDs();

        // Ensure all spriteIDs are -1 if not valid, or the next available ID if valid
        int spriteIDCounter = 0;
        std::vector<enginetype::GridCell> allSprites = testutil::FULL_LEVEL.allSprites();
        for (int x = 0; x < levelWidth; x++) {
            for (int y = 0; y < levelHeight; y++) {
                int spriteID = gridinfo::getSpriteID({x,y});
                bool inAllSprites = std::find(allSprites.begin(), allSprites.end(), (enginetype::GridCell){x,y}) != allSprites.end();
                bool isValid = (spriteID == -1 && !inAllSprites) || (spriteID == spriteIDCounter++ && inAllSprites);
                REQUIRE(isValid);
            }
        }
        // Out of bounds
        REQUIRE(gridinfo::getSpriteID({-1,-1}) == -1);
        REQUIRE(gridinfo::getSpriteID({levelWidth, levelHeight}) == -1);
    }
}


/**
 * Test for getting the sprite grid cell
 */
TEST_CASE("Check the sprite gridcell", "[gridcell_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Initialize the sprite IDs
    gridinfo::initSpriteIDs();

    // Check known sprite ID and compare
    enginetype::GridCell goalCell = testutil::FULL_LEVEL.door[0];
    int goalSpriteID = gridinfo::getSpriteID(goalCell);
    REQUIRE(gridinfo::getSpriteGridCell(goalSpriteID) == goalCell);
}


/**
 * Test for checking whether sprites are active
 */
TEST_CASE("Check for active sprites", "[gridcell_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    int levelWidth = levelinfo::getLevelWidth();
    int levelHeight = levelinfo::getLevelHeight();
    gridinfo::initSpriteIDs();

    // Invalid sprite
    REQUIRE(!gridinfo::isSpriteActive(-1));

    // Check if sprite active and in known sprites, or invalid and not known
    std::vector<enginetype::GridCell> allSprites = testutil::FULL_LEVEL.allSprites();
    for (int x = 0; x < levelWidth; x++) {
        for (int y = 0; y < levelHeight; y++) {
            enginetype::GridCell cell = {x, y};
            int spriteID = gridinfo::getSpriteID(cell);
            bool inAllSprites = std::find(allSprites.begin(), allSprites.end(), cell) != allSprites.end();
            REQUIRE(gridinfo::isSpriteActive(spriteID) == inAllSprites);
        }
    }
}


/**
 * Test for sprite ID initialization and retrieving them.
 */
TEST_CASE("Getting the map sprite list", "[gridcell_information]") {
    SECTION("Empty level with empty sprite list") {
        int levelNum = testutil::EMPTY_LEVEL.levelNum;
        testutil::loadTestLevelAndStart(levelNum);

        // Initialize the sprite IDs
        gridinfo::initSpriteIDs();

        // Empty map should have an empty sprite list
        std::vector<enginetype::GridCell> mapSprites = gridinfo::getMapSprites();
        REQUIRE(mapSprites.size() == testutil::EMPTY_LEVEL.numSprites());
    }
    SECTION("Full level with populated sprite list") {
        int levelNum = testutil::FULL_LEVEL.levelNum;
        testutil::loadTestLevelAndStart(levelNum);

        // Initialize the sprite IDs
        gridinfo::initSpriteIDs();

        // Empty map should have an empty sprite list
        std::vector<enginetype::GridCell> mapSprites = gridinfo::getMapSprites();
        REQUIRE(mapSprites.size() == testutil::FULL_LEVEL.numSprites());
    }
}