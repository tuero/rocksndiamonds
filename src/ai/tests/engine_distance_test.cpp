/**
 * @file: engine_distance_test.cpp
 *
 * @brief: Test functions for the distance section of the engine helper
 * 
 * @author: Jake Tuero
 * Date: December 2019
 * Contact: tuero@ualberta.ca
 */


// Testing framework
#include <catch2/catch.hpp>

// Standard Libary/STL
#include <cmath>                // abs

// Includes
#include "engine_types.h"
#include "engine_helper.h"

// Testing util
#include "test_util.h"


/**
 * Test the getL1Distance function 
 */
TEST_CASE("Get L1 Distance", "[distance]") {
    SECTION("Same cell to compare") {
        REQUIRE(enginehelper::getL1Distance({1,1}, {1,1}) == 0);
    }
    SECTION("Cells in same row/column") {
        REQUIRE(enginehelper::getL1Distance({1,1}, {6,1}) == 5);
        REQUIRE(enginehelper::getL1Distance({1,1}, {1,6}) == 5);
    }
    SECTION("Cells on diagonal") {
        REQUIRE(enginehelper::getL1Distance({1,1}, {6,6}) == 5+5);
        REQUIRE(enginehelper::getL1Distance({1,1}, {5,6}) == 5+4);
    }
}


/**
 * Test the settting and getting of grid distance to goal function 
 */
TEST_CASE("Get and Set Individual Grid Distances", "[distance]") {
    int levelNum = testutil::EMPTY_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    int levelWidth = enginehelper::getLevelWidth();
    int levelHeight = enginehelper::getLevelHeight();
    enginetype::GridCell referenceCell = {(int)levelWidth / 2, (int)levelHeight / 2};
    
    int firstDistanceTest = 1;
    enginehelper::setGridDistanceToGoal({0,0}, 0);
    enginehelper::setGridDistanceToGoal(referenceCell, firstDistanceTest);
    REQUIRE(enginehelper::getGridDistanceToGoal({0,0}) == 0);
    REQUIRE(enginehelper::getGridDistanceToGoal(referenceCell) == firstDistanceTest);

    // Change again and ensure other cells are unchanged
    int secondDistanceTest = 10;
    enginehelper::setGridDistanceToGoal(referenceCell, secondDistanceTest);
    REQUIRE(enginehelper::getGridDistanceToGoal({0,0}) == 0);
    REQUIRE(enginehelper::getGridDistanceToGoal(referenceCell) == secondDistanceTest);
}


/**
 * Test the resetBoardDistances function 
 */
TEST_CASE("Reset the distances", "[distance]") {
    int levelNum = testutil::EMPTY_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    int levelWidth = enginehelper::getLevelWidth();
    int levelHeight = enginehelper::getLevelHeight();

    // Set distance values
    enginetype::GridCell goalCell = {(int)levelWidth / 2, (int)levelHeight / 2};
    enginehelper::setBoardDistancesL1(goalCell);

    // Check goalcell has distance to goal of 0
    REQUIRE(enginehelper::getGridDistanceToGoal(goalCell) == 0);

    // Reset distances and check they were reset
    enginehelper::resetBoardDistances();
    for (int x = 0; x < levelWidth; x++) {
        for (int y = 0; y < levelHeight; y++) {
            REQUIRE(enginehelper::getGridDistanceToGoal(goalCell) == -1);
        }
    }
}


/**
 * Test the setBoardDistancesL1 function 
 */
TEST_CASE("Setting goal distance", "[distance]") {
    int levelNum = testutil::EMPTY_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    int levelWidth = enginehelper::getLevelWidth();
    int levelHeight = enginehelper::getLevelHeight();
    enginetype::GridCell goalCell = {(int)levelWidth / 2, (int)levelHeight / 2};

    // Set goal distance
    enginehelper::resetBoardDistances();
    enginehelper::setBoardDistancesL1(goalCell);

    // Test to ensure all distances match
    for (int x = 0; x < levelWidth; x++) {
        for (int y = 0; y < levelHeight; y++) {
            int distance = std::abs(goalCell.x - x) + std::abs(goalCell.y - y);
            REQUIRE(enginehelper::getGridDistanceToGoal({x, y}) == distance);
        }
    }

}

/**
 * Test the findDistanceGoalCell function 
 */
TEST_CASE("Finding the distance goal location", "[distance]") {
    int levelNum = testutil::EMPTY_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    int levelWidth = enginehelper::getLevelWidth();
    int levelHeight = enginehelper::getLevelHeight();
    enginetype::GridCell goalCell = {(int)levelWidth / 2, (int)levelHeight / 2};

    // No goal set
    enginehelper::resetBoardDistances();
    REQUIRE(enginehelper::findDistanceGoalCell() == (enginetype::GridCell){-1, -1});

    // Goal set
    enginehelper::setBoardDistancesL1(goalCell);
    REQUIRE(enginehelper::findDistanceGoalCell() == goalCell);
}


/**
 * Test the findDistanceGoalCell function 
 */
TEST_CASE("Finding the exit location", "[distance]") {
    SECTION("Level with no exit") {
        int levelNum = testutil::EMPTY_LEVEL.levelNum;
        testutil::loadTestLevelAndStart(levelNum);
        REQUIRE(enginehelper::findExitLocation() == (enginetype::GridCell){-1, -1});
    }
    SECTION("Level with no exit") {
        int levelNum = testutil::FULL_LEVEL.levelNum;
        testutil::loadTestLevelAndStart(levelNum);
        REQUIRE(enginehelper::findExitLocation() == testutil::FULL_LEVEL.door[0]);
    }
}