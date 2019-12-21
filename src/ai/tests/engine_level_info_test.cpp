/**
 * @file: engine_level_info_test.cpp
 *
 * @brief: Test functions for the level information section of the engine helper
 * 
 * @author: Jake Tuero
 * Date: December 2019
 * Contact: tuero@ualberta.ca
 */

// Testing framework
#include <catch2/catch.hpp>

// Standard Libary/STL
#include <vector>

// Includes
#include "engine_helper.h"

// Test util
#include "test_util.h"


/**
 * Test the level information functions
 */
TEST_CASE("Loading the default test level/levelset and check attribtues", "[level_information]") {
    int levelWidth = testutil::FULL_LEVEL.levelWidth;
    int levelHeight = testutil::FULL_LEVEL.levelHeight;
    int gemsNeeded = testutil::FULL_LEVEL.numGemsNeeded;

    // Set levelset and load level
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Start game
    StartGameActions(network.enabled, setup.autorecord, level.random_seed);

    // Check level number
    REQUIRE(enginehelper::getLevelNumber() == levelNum);

    // Check level dimensions
    REQUIRE(enginehelper::getLevelWidth() == levelWidth);
    REQUIRE(enginehelper::getLevelHeight() == levelHeight);

    // Check gems to collect
    REQUIRE(enginehelper::getLevelGemsNeeded() == gemsNeeded);
    REQUIRE(enginehelper::getLevelRemainingGemsNeeded() == gemsNeeded);

    // Collect a diamond and then check the updated gem count
    enginehelper::setEnginePlayerAction(Action::right);
    enginehelper::engineSimulate();
    enginehelper::setEnginePlayerAction(Action::right);
    enginehelper::engineSimulate();
    REQUIRE(enginehelper::getLevelGemsNeeded() == gemsNeeded);
    REQUIRE(enginehelper::getLevelRemainingGemsNeeded() == gemsNeeded - 1);
}
