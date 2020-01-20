/**
 * @file: engine_element_info_test.cpp
 *
 * @brief: Test functions for the element info section of the engine helper
 * 
 * @author: Jake Tuero
 * Date: December 2019
 * Contact: tuero@ualberta.ca
 */

// Testing framework
#include <catch2/catch.hpp>

// Includes
#include "engine_types.h"
#include "engine_helper.h"

// Testing util
#include "test_util.h"


/**
 * Test for item readable description
 */
TEST_CASE("Check Item Readable Description", "[element_information]") {
    // Known elements
    for (auto const & cell : testutil::FULL_LEVEL.allSprites()) {
        int item = enginehelper::getGridElement(cell);
        REQUIRE(enginehelper::getItemReadableDescription(item) != "UNKNOWN");
    }

    // Unknown elements
    REQUIRE(enginehelper::getItemReadableDescription(enginetype::NUM_ELEMENTS) == "UNKNOWN");
}


/**
 * Test for various gem counts
 */
TEST_CASE("Check Item Gem Count", "[element_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Check each gem type
    for (auto const & cell : testutil::FULL_LEVEL.diamonds_bd) {
        REQUIRE(enginehelper::getItemGemCount(cell) == 1);
    }
    for (auto const & cell : testutil::FULL_LEVEL.diamonds_em) {
        REQUIRE(enginehelper::getItemGemCount(cell) == 3);
    }
    for (auto const & cell : testutil::FULL_LEVEL.emerald) {
        REQUIRE(enginehelper::getItemGemCount(cell) == 1);
    }
    
    // Check non-gem
    REQUIRE(enginehelper::getItemGemCount(testutil::FULL_LEVEL.door[0]) == 0);

    // Check unknown type
    REQUIRE(enginehelper::getItemGemCount({-1, -1}) == 0);
}


/**
 * Test for various item scores
 */
TEST_CASE("Check Item Score", "[element_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Check each gem type
    for (auto const & cell : testutil::FULL_LEVEL.diamonds_bd) {
        REQUIRE(enginehelper::getItemScore(cell) == 10);
    }
    for (auto const & cell : testutil::FULL_LEVEL.diamonds_em) {
        REQUIRE(enginehelper::getItemScore(cell) == 10);
    }
    for (auto const & cell : testutil::FULL_LEVEL.emerald) {
        REQUIRE(enginehelper::getItemScore(cell) == 10);
    }

    for (auto const & cell : testutil::FULL_LEVEL.custom) {
        REQUIRE(enginehelper::getItemScore(cell) == 20);
    }
    
    // Check non-gem
    REQUIRE(enginehelper::getItemScore(testutil::FULL_LEVEL.door[0]) == 0);

    // Check unknown type
    REQUIRE(enginehelper::getItemScore({-1, -1}) == 0);
}


/**
 * Test for temporary element detection
 */
TEST_CASE("Check Temporary Element", "[element_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Temporary element is placed under falling object while it is in motion to that grid
    for (auto const & cell : testutil::FULL_LEVEL.allRocks()) {
        REQUIRE(!enginehelper::isTemporaryElement((enginetype::GridCell){cell.x, cell.y+1}));
    }
    
    // Progress world 1 step forward, rocks initiate falling and put a claim on grid below
    enginehelper::setEnginePlayerAction(Action::noop);
    enginehelper::engineSimulateSingle();
    for (auto const & cell : testutil::FULL_LEVEL.allRocks()) {
        REQUIRE(enginehelper::isTemporaryElement((enginetype::GridCell){cell.x, cell.y+1}));
    }
}


/**
 * Test for item moving
 */
TEST_CASE("Check Item Moving", "[element_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Temporary element is placed under falling object while it is in motion to that grid
    for (auto const & cell : testutil::FULL_LEVEL.allRocks()) {
        REQUIRE(!enginehelper::isMoving(cell));
    }
    
    // Progress world 1 step forward, rocks initiate falling and put a claim on grid below
    enginehelper::setEnginePlayerAction(Action::noop);
    enginehelper::engineSimulateSingle();
    for (auto const & cell : testutil::FULL_LEVEL.allRocks()) {
        REQUIRE(enginehelper::isMoving(cell));
    }
}


// Door status flags
const uint16_t CLOSED_FLAG  = (1 << 0);
const uint16_t OPENING_FLAG = (1 << 1);
const uint16_t OPEN_FLAG    = (1 << 2);
const uint16_t CLOSING_FLAG = (1 << 3);


void _checkDoorStatuses(const enginetype::GridCell &door, uint16_t status) {
    REQUIRE(enginehelper::isExit(door));        // Door is always set
    REQUIRE(enginehelper::isExitClosed(door) == (bool)(status & CLOSED_FLAG));
    REQUIRE(enginehelper::isExitOpening(door) == (bool)(status & OPENING_FLAG));
    REQUIRE(enginehelper::isExitOpen(door) == (bool)(status & OPEN_FLAG));
    REQUIRE(enginehelper::isExitClosing(door) == (bool)(status & CLOSING_FLAG));
}


/**
 * Test for various door statuses
 */
TEST_CASE("Check Door Statuses", "[element_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Initial check
    enginetype::GridCell door = testutil::FULL_LEVEL.door[0];
    _checkDoorStatuses(door, 0 | CLOSED_FLAG);

    // Move right picking up diamonds
    for (int i = 0; i < 5*enginetype::ENGINE_RESOLUTION; i++) {
        enginehelper::setEnginePlayerAction(Action::right);
        enginehelper::engineSimulateSingle();
        _checkDoorStatuses(door, 0 | CLOSED_FLAG);
    }

    // Right beside final diamond, initiate single tick to start final collection process
    enginehelper::setEnginePlayerAction(Action::right);
    enginehelper::engineSimulateSingle();
    _checkDoorStatuses(door, 0 | OPENING_FLAG);

    // Once opening sequence initiated, it takes DOOR_CHANGE_DELAY ticks to complete the opening sequence.
    for (int i = 0; i < enginetype::DOOR_CHANGE_DELAY; i++) {
        enginehelper::setEnginePlayerAction(Action::noop);
        enginehelper::engineSimulateSingle();
        _checkDoorStatuses(door, 0 | OPENING_FLAG);
    }

    // Door is now open, move towards goal
    for (int i = 0; i < 9*enginetype::ENGINE_RESOLUTION; i++) {
        enginehelper::setEnginePlayerAction(Action::right);
        enginehelper::engineSimulateSingle();
        _checkDoorStatuses(door, 0 | OPEN_FLAG);
    }

    // We are now inside the exit 
    // Progress one tick and check for door closing
    enginehelper::setEnginePlayerAction(Action::noop);
    enginehelper::engineSimulateSingle();
    _checkDoorStatuses(door, 0 | CLOSING_FLAG);
}


/**
 * Check elements collectible status 
 */
TEST_CASE("Check Collectible Statuses", "[element_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Gems should be collectible
    for (auto const & cell : testutil::FULL_LEVEL.allGems()) {
        REQUIRE(enginehelper::isCollectable(cell));
    }

    // Keys should be collectible
    for (auto const & cell : testutil::FULL_LEVEL.keys) {
        REQUIRE(enginehelper::isCollectable(cell));
    }

    // Rocks are not collectible
    for (auto const & cell : testutil::FULL_LEVEL.allRocks()) {
        REQUIRE(!enginehelper::isCollectable(cell));
    }
    
}


/**
 * Check gate status with and without keys
 */
TEST_CASE("Check Gate Statuses", "[element_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Check gates
    for (auto const & cell : testutil::FULL_LEVEL.gates) {
        REQUIRE(enginehelper::isGate(cell));
    }

    // Gate shouldn't be open
    for (auto const & cell : testutil::FULL_LEVEL.gates) {
        REQUIRE(enginehelper::isGateClosed(cell));
        REQUIRE(!enginehelper::isGateOpen(cell));
    }

    // Walk to key
    for (int i = 0; i < 11; i++) {
        enginehelper::setEnginePlayerAction(Action::right);
        enginehelper::engineSimulate();
    }
    for (int i = 0; i < 12; i++) {
        enginehelper::setEnginePlayerAction(Action::down);
        enginehelper::engineSimulate();
    }
    for (int i = 0; i < 8; i++) {
        enginehelper::setEnginePlayerAction(Action::left);
        enginehelper::engineSimulate();
    }

    // Gate shouldn't be open
    for (auto const & cell : testutil::FULL_LEVEL.gates) {
        REQUIRE(enginehelper::isGateClosed(cell));
        REQUIRE(!enginehelper::isGateOpen(cell));
    }

    // Walk over key
    enginehelper::setEnginePlayerAction(Action::down);
    enginehelper::engineSimulate();
    

    // Gate should now be open
    for (int i = 0; i < testutil::FULL_LEVEL.numGates(); i++) {
        enginetype::GridCell cell = testutil::FULL_LEVEL.gates[i];
        if (i == 1) {
            REQUIRE(!enginehelper::isGateClosed(cell));
            REQUIRE(enginehelper::isGateOpen(cell));
        }
        else {
            REQUIRE(enginehelper::isGateClosed(cell));
            REQUIRE(!enginehelper::isGateOpen(cell));
        }
    }
   
}