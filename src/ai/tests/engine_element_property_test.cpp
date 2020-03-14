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

using namespace enginehelper;


/**
 * Test for item readable description
 */
TEST_CASE("Check Item Readable Description", "[element_information]") {
    // Known elements
    for (auto const & cell : testutil::FULL_LEVEL.allSprites()) {
        int item = gridinfo::getGridElement(cell);
        REQUIRE(elementproperty::getItemReadableDescription(item) != "UNKNOWN");
    }

    // Unknown elements
    REQUIRE(elementproperty::getItemReadableDescription(enginetype::NUM_ELEMENTS) == "UNKNOWN");
}


/**
 * Test for various gem counts
 */
TEST_CASE("Check Item Gem Count", "[element_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Check each gem type
    for (auto const & cell : testutil::FULL_LEVEL.diamonds_bd) {
        REQUIRE(elementproperty::getItemGemCount(cell) == 1);
    }
    for (auto const & cell : testutil::FULL_LEVEL.diamonds_em) {
        REQUIRE(elementproperty::getItemGemCount(cell) == 3);
    }
    for (auto const & cell : testutil::FULL_LEVEL.emerald) {
        REQUIRE(elementproperty::getItemGemCount(cell) == 1);
    }
    
    // Check non-gem
    REQUIRE(elementproperty::getItemGemCount(testutil::FULL_LEVEL.door[0]) == 0);

    // Check unknown type
    REQUIRE(elementproperty::getItemGemCount({-1, -1}) == 0);
}


/**
 * Test for various item scores
 */
TEST_CASE("Check Item Score", "[element_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Check each gem type
    for (auto const & cell : testutil::FULL_LEVEL.diamonds_bd) {
        REQUIRE(elementproperty::getItemScore(cell) == 10);
    }
    for (auto const & cell : testutil::FULL_LEVEL.diamonds_em) {
        REQUIRE(elementproperty::getItemScore(cell) == 10);
    }
    for (auto const & cell : testutil::FULL_LEVEL.emerald) {
        REQUIRE(elementproperty::getItemScore(cell) == 10);
    }

    for (auto const & cell : testutil::FULL_LEVEL.custom) {
        REQUIRE(elementproperty::getItemScore(cell) == 20);
    }
    
    // Check non-gem
    REQUIRE(elementproperty::getItemScore(testutil::FULL_LEVEL.door[0]) == 0);

    // Check unknown type
    REQUIRE(elementproperty::getItemScore({-1, -1}) == 0);
}


/**
 * Test for temporary element detection
 */
TEST_CASE("Check Temporary Element", "[element_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Temporary element is placed under falling object while it is in motion to that grid
    for (auto const & cell : testutil::FULL_LEVEL.allRocks()) {
        REQUIRE(!elementproperty::isTemporaryElement((enginetype::GridCell){cell.x, cell.y+1}));
    }
    
    // Progress world 1 step forward, rocks initiate falling and put a claim on grid below
    enginestate::setEnginePlayerAction(Action::noop);
    enginestate::engineSimulateSingle();
    for (auto const & cell : testutil::FULL_LEVEL.allRocks()) {
        REQUIRE(elementproperty::isTemporaryElement((enginetype::GridCell){cell.x, cell.y+1}));
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
        REQUIRE(!elementproperty::isMoving(cell));
    }
    
    // Progress world 1 step forward, rocks initiate falling and put a claim on grid below
    enginestate::setEnginePlayerAction(Action::noop);
    enginestate::engineSimulateSingle();
    for (auto const & cell : testutil::FULL_LEVEL.allRocks()) {
        REQUIRE(elementproperty::isMoving(cell));
    }
}


// Door status flags
const uint16_t CLOSED_FLAG  = (1 << 0);
const uint16_t OPENING_FLAG = (1 << 1);
const uint16_t OPEN_FLAG    = (1 << 2);
const uint16_t CLOSING_FLAG = (1 << 3);


void _checkDoorStatuses(const enginetype::GridCell &door, uint16_t status) {
    REQUIRE(elementproperty::isExit(door));        // Door is always set
    REQUIRE(elementproperty::isExitClosed(door) == (bool)(status & CLOSED_FLAG));
    REQUIRE(elementproperty::isExitOpening(door) == (bool)(status & OPENING_FLAG));
    REQUIRE(elementproperty::isExitOpen(door) == (bool)(status & OPEN_FLAG));
    REQUIRE(elementproperty::isExitClosing(door) == (bool)(status & CLOSING_FLAG));
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
    for (int i = 0; i < 5*enginestate::getEngineUpdateRate(); i++) {
        enginestate::setEnginePlayerAction(Action::right);
        enginestate::engineSimulateSingle();
        _checkDoorStatuses(door, 0 | CLOSED_FLAG);
    }

    // Right beside final diamond, initiate single tick to start final collection process
    enginestate::setEnginePlayerAction(Action::right);
    enginestate::engineSimulateSingle();
    _checkDoorStatuses(door, 0 | OPENING_FLAG);

    // Once opening sequence initiated, it takes DOOR_CHANGE_DELAY ticks to complete the opening sequence.
    for (int i = 0; i < enginetype::DOOR_CHANGE_DELAY; i++) {
        enginestate::setEnginePlayerAction(Action::noop);
        enginestate::engineSimulateSingle();
        _checkDoorStatuses(door, 0 | OPENING_FLAG);
    }

    // Door is now open, move towards goal
    for (int i = 0; i < 9*enginestate::getEngineUpdateRate(); i++) {
        enginestate::setEnginePlayerAction(Action::right);
        enginestate::engineSimulateSingle();
        _checkDoorStatuses(door, 0 | OPEN_FLAG);
    }

    // We are now inside the exit 
    // Progress one tick and check for door closing
    enginestate::setEnginePlayerAction(Action::noop);
    enginestate::engineSimulateSingle();
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
        REQUIRE(elementproperty::isCollectable(cell));
    }

    // Keys should be collectible
    for (auto const & cell : testutil::FULL_LEVEL.keys) {
        REQUIRE(elementproperty::isCollectable(cell));
    }

    // Rocks are not collectible
    for (auto const & cell : testutil::FULL_LEVEL.allRocks()) {
        REQUIRE(!elementproperty::isCollectable(cell));
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
        REQUIRE(elementproperty::isGate(cell));
    }

    // Gate shouldn't be open
    for (auto const & cell : testutil::FULL_LEVEL.gates) {
        REQUIRE(elementproperty::isGateClosed(cell));
        REQUIRE(!elementproperty::isGateOpen(cell));
    }

    // Walk to key
    for (int i = 0; i < 11; i++) {
        enginestate::setEnginePlayerAction(Action::right);
        enginestate::engineSimulate();
    }
    for (int i = 0; i < 12; i++) {
        enginestate::setEnginePlayerAction(Action::down);
        enginestate::engineSimulate();
    }
    for (int i = 0; i < 8; i++) {
        enginestate::setEnginePlayerAction(Action::left);
        enginestate::engineSimulate();
    }

    // Gate shouldn't be open
    for (auto const & cell : testutil::FULL_LEVEL.gates) {
        REQUIRE(elementproperty::isGateClosed(cell));
        REQUIRE(!elementproperty::isGateOpen(cell));
    }

    // Walk over key
    enginestate::setEnginePlayerAction(Action::down);
    enginestate::engineSimulate();
    

    // Gate should now be open
    for (int i = 0; i < testutil::FULL_LEVEL.numGates(); i++) {
        enginetype::GridCell cell = testutil::FULL_LEVEL.gates[i];
        if (i == 1) {
            REQUIRE(!elementproperty::isGateClosed(cell));
            REQUIRE(elementproperty::isGateOpen(cell));
        }
        else {
            REQUIRE(elementproperty::isGateClosed(cell));
            REQUIRE(!elementproperty::isGateOpen(cell));
        }
    }
   
}


/**
 * Check gate status with and without keys
 */
TEST_CASE("Check Walkable Property", "[element_information]") {
    int levelNum = testutil::FULL_LEVEL.levelNum;
    testutil::loadTestLevelAndStart(levelNum);

    // Cant walk or dig gem, but can collect it
    enginetype::GridCell gemCell = testutil::FULL_LEVEL.diamonds_bd[0];
    enginetype::GridCell cellFromGem = gridaction::getCellFromAction(gemCell, Action::left);
    REQUIRE(!elementproperty::isWalkable(cellFromGem, Action::right));
    REQUIRE(!elementproperty::isDigable(cellFromGem, Action::right));
    REQUIRE(elementproperty::isCollectable(cellFromGem, Action::right));
    REQUIRE(!elementproperty::isRock(cellFromGem, Action::right));
    REQUIRE(!elementproperty::isPassable(cellFromGem, Action::right));
    REQUIRE(elementproperty::isActionMoveable(cellFromGem, Action::right));
    REQUIRE(!elementproperty::isPushable(cellFromGem, Action::right));

    // Cant dig or collect empty space, but can walk
    enginetype::GridCell playerCell = gridinfo::getPlayerPosition();
    REQUIRE(elementproperty::isWalkable(playerCell, Action::right));
    REQUIRE(!elementproperty::isDigable(playerCell, Action::right));
    REQUIRE(!elementproperty::isCollectable(playerCell, Action::right));
    REQUIRE(!elementproperty::isRock(playerCell, Action::right));
    REQUIRE(!elementproperty::isPassable(playerCell, Action::right));
    REQUIRE(elementproperty::isActionMoveable(playerCell, Action::right));
    REQUIRE(!elementproperty::isPushable(playerCell, Action::right));

    // Cant walk or collect dirt, but is diggable
    REQUIRE(!elementproperty::isWalkable(playerCell, Action::down));
    REQUIRE(elementproperty::isDigable(playerCell, Action::down));
    REQUIRE(!elementproperty::isCollectable(playerCell, Action::down));
    REQUIRE(!elementproperty::isRock(playerCell, Action::right));
    REQUIRE(!elementproperty::isPassable(playerCell, Action::right));
    REQUIRE(elementproperty::isActionMoveable(playerCell, Action::right));
    REQUIRE(!elementproperty::isPushable(playerCell, Action::right));

    // Check rock is not walkable or collectible
    enginetype::GridCell rockCell = testutil::FULL_LEVEL.rock_bd[0];
    enginetype::GridCell cellFromRock = gridaction::getCellFromAction(rockCell, Action::left);
    REQUIRE(elementproperty::isRock(cellFromRock, Action::right));
    REQUIRE(!elementproperty::isWalkable(cellFromRock, Action::right));
    REQUIRE(!elementproperty::isDigable(cellFromRock, Action::right));
    REQUIRE(!elementproperty::isCollectable(cellFromRock, Action::right));
    REQUIRE(!elementproperty::isPassable(cellFromRock, Action::right));
    REQUIRE(!elementproperty::isActionMoveable(cellFromRock, Action::right));
    REQUIRE(elementproperty::isPushable(cellFromRock, Action::right));

    // Check if the port tube is passable, walkable if inside the tube, and not if blocked
    enginetype::GridCell portCellUnblocked = testutil::FULL_LEVEL.misc[1];
    enginetype::GridCell portCellBlocked = testutil::FULL_LEVEL.misc[2];
    enginetype::GridCell cellFromUnblocked = gridaction::getCellFromAction(portCellUnblocked, Action::left);
    enginetype::GridCell cellFromBlocked = gridaction::getCellFromAction(portCellBlocked, Action::left);
    REQUIRE(elementproperty::isPassable(cellFromUnblocked, Action::right));
    REQUIRE(!elementproperty::isPassable(cellFromBlocked, Action::right));
    REQUIRE(elementproperty::isWalkable(portCellUnblocked, Action::right));
    REQUIRE(!elementproperty::isWalkable(portCellBlocked, Action::right));
    REQUIRE(elementproperty::isActionMoveable(cellFromUnblocked, Action::right));
    REQUIRE(elementproperty::isActionMoveable(portCellUnblocked, Action::right));
    REQUIRE(!elementproperty::isActionMoveable(cellFromBlocked, Action::right));
    REQUIRE(!elementproperty::isActionMoveable(portCellBlocked, Action::right));
    REQUIRE(!elementproperty::isPushable(cellFromUnblocked, Action::right));
    REQUIRE(!elementproperty::isPushable(cellFromBlocked, Action::right));
    REQUIRE(!elementproperty::isPushable(portCellUnblocked, Action::right));
    REQUIRE(!elementproperty::isPushable(portCellBlocked, Action::right));
}