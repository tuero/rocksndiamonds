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


/**
 * Test the actionToString function 
 */
TEST_CASE("Action to string", "[action_information]") {
    SECTION("Unknown action type") {
        REQUIRE(enginehelper::actionToString(static_cast<Action>(-1)) == "noop");
    }
    SECTION("Known action types") {
        REQUIRE(enginehelper::actionToString(Action::up) == "up");
        REQUIRE(enginehelper::actionToString(Action::down) == "down");
        REQUIRE(enginehelper::actionToString(Action::left) == "left");
        REQUIRE(enginehelper::actionToString(Action::right) == "right");
        REQUIRE(enginehelper::actionToString(Action::noop) == "noop");
    }
}


/**
 * Test the stringToAction function 
 */
TEST_CASE("String to action", "[action_information]") {
    SECTION("Unknown action type") {
        REQUIRE(enginehelper::stringToAction("unkown") == Action::noop);
    }
    SECTION("Known action types") {
        REQUIRE(enginehelper::stringToAction("up") ==Action::up);
        REQUIRE(enginehelper::stringToAction("down") == Action::down);
        REQUIRE(enginehelper::stringToAction("left") == Action::left);
        REQUIRE(enginehelper::stringToAction("right") == Action::right);
        REQUIRE(enginehelper::stringToAction("noop") == Action::noop);
    }
}


/**
 * Test the getOppositeDirection function 
 */
TEST_CASE("Get action opposite direction", "[action_information]") {
    SECTION("Unknown action type") {
        REQUIRE(enginehelper::getOppositeDirection(static_cast<Action>(-1)) == Action::noop);
    }
    SECTION("Known action types") {
        REQUIRE(enginehelper::getOppositeDirection(Action::up) == Action::down);
        REQUIRE(enginehelper::getOppositeDirection(Action::down) == Action::up);
        REQUIRE(enginehelper::getOppositeDirection(Action::left) == Action::right);
        REQUIRE(enginehelper::getOppositeDirection(Action::right) == Action::left);
        REQUIRE(enginehelper::getOppositeDirection(Action::noop) == Action::noop);
    }
}