/**
 * @file: engine_action_info_test.cpp
 *
 * @brief: Test functions for the action information section of the engine helper
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


using namespace enginehelper;

/**
 * Test the actionToString function 
 */
TEST_CASE("Action to string", "[action_information]") {
    SECTION("Unknown action type") {
        REQUIRE(actioninfo::actionToString(static_cast<Action>(-1)) == "noop");
    }
    SECTION("Known action types") {
        REQUIRE(actioninfo::actionToString(Action::up) == "up");
        REQUIRE(actioninfo::actionToString(Action::down) == "down");
        REQUIRE(actioninfo::actionToString(Action::left) == "left");
        REQUIRE(actioninfo::actionToString(Action::right) == "right");
        REQUIRE(actioninfo::actionToString(Action::noop) == "noop");
    }
}


/**
 * Test the stringToAction function 
 */
TEST_CASE("String to action", "[action_information]") {
    SECTION("Unknown action type") {
        REQUIRE(actioninfo::stringToAction("unkown") == Action::noop);
    }
    SECTION("Known action types") {
        REQUIRE(actioninfo::stringToAction("up") ==Action::up);
        REQUIRE(actioninfo::stringToAction("down") == Action::down);
        REQUIRE(actioninfo::stringToAction("left") == Action::left);
        REQUIRE(actioninfo::stringToAction("right") == Action::right);
        REQUIRE(actioninfo::stringToAction("noop") == Action::noop);
    }
}


/**
 * Test the getOppositeDirection function 
 */
TEST_CASE("Get action opposite direction", "[action_information]") {
    SECTION("Unknown action type") {
        REQUIRE(actioninfo::getOppositeDirection(static_cast<Action>(-1)) == Action::noop);
    }
    SECTION("Known action types") {
        REQUIRE(actioninfo::getOppositeDirection(Action::up) == Action::down);
        REQUIRE(actioninfo::getOppositeDirection(Action::down) == Action::up);
        REQUIRE(actioninfo::getOppositeDirection(Action::left) == Action::right);
        REQUIRE(actioninfo::getOppositeDirection(Action::right) == Action::left);
        REQUIRE(actioninfo::getOppositeDirection(Action::noop) == Action::noop);
    }
}