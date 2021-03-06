/**
 * @file: engine_grid_action_test.cpp
 *
 * @brief: Test functions for the grid action section of the engine helper
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
#include "engine_types.h"
#include "engine_helper.h"

// Game headers
extern "C" {
    #include "../../main.h"
}

using namespace enginehelper;


/**
 * Test the checkIfNeighbours function 
 */
TEST_CASE("Check if neighbours", "[grid_action]") {
    int LEVEL_WIDTH = 10;
    int LEVEL_HEIGHT = 10;

    // set level boundaries
    level.fieldx = LEVEL_WIDTH;
    level.fieldy = LEVEL_HEIGHT;

    SECTION("Both cells out of bounds") {
        REQUIRE(!gridaction::checkIfNeighbours({-1, -1}, {0, 0}));
    }
    SECTION("One cell in bounds, the other is not") {
        REQUIRE(!gridaction::checkIfNeighbours({-1, -1}, {0, 0}));
        REQUIRE(!gridaction::checkIfNeighbours({LEVEL_WIDTH-1, LEVEL_HEIGHT-1}, {LEVEL_WIDTH, LEVEL_HEIGHT}));
    }
    SECTION("Both cells in bounds, not neighbours") {
        // More than 1 tile away
        REQUIRE(!gridaction::checkIfNeighbours({2, 2}, {0, 0}));
        REQUIRE(!gridaction::checkIfNeighbours({0, 0}, {2, 2}));

        // On the diagonal
        REQUIRE(!gridaction::checkIfNeighbours({2, 2}, {1, 1}));
        REQUIRE(!gridaction::checkIfNeighbours({1, 1}, {2, 2}));

        // Same cell
        REQUIRE(!gridaction::checkIfNeighbours({1, 1}, {1, 1}));
    }
    SECTION("Both cells in bounds, neighbours") {
        enginetype::GridCell referenceCell = {2, 2};
        std::vector<enginetype::GridCell> neighbours = {{referenceCell.x, referenceCell.y - 1}, {referenceCell.x, referenceCell.y + 1},
            {referenceCell.x - 1, referenceCell.y}, {referenceCell.x + 1, referenceCell.y}};
        for (auto const & cell : neighbours) {
            REQUIRE(gridaction::checkIfNeighbours(referenceCell, cell));
            REQUIRE(gridaction::checkIfNeighbours(cell, referenceCell));
        }
    }
}


/**
 * Test the getActionFromNeighbours function 
 */
TEST_CASE("Get action from neighbours", "[grid_action]") {
    enginetype::GridCell referenceCell = {2, 2};
    SECTION("Cells not neighbours") {
        // More than 1 tile away
        REQUIRE(gridaction::getActionFromNeighbours({2, 2}, {0, 0}) == Action::noop);
        REQUIRE(gridaction::getActionFromNeighbours({0, 0}, {2, 2}) == Action::noop);

        // On the diagonal
        REQUIRE(gridaction::getActionFromNeighbours({2, 2}, {1, 1}) == Action::noop);
        REQUIRE(gridaction::getActionFromNeighbours({1, 1}, {2, 2}) == Action::noop);
    }
    SECTION("Both cells the same") {
        REQUIRE(gridaction::getActionFromNeighbours(referenceCell, referenceCell) == Action::noop);
        REQUIRE(gridaction::getActionFromNeighbours(referenceCell, referenceCell) == Action::noop);
    }
    SECTION("Cells are neighbours") {
        std::vector<enginetype::GridCell> neighbours = {{referenceCell.x - 1, referenceCell.y}, {referenceCell.x + 1, referenceCell.y},
            {referenceCell.x, referenceCell.y - 1}, {referenceCell.x, referenceCell.y + 1}};
        std::vector<Action> actionsForward = {Action::left, Action::right, Action::up, Action::down};
        std::vector<Action> actionsBackward = {Action::right, Action::left, Action::down, Action::up};
        REQUIRE(neighbours.size() == actionsForward.size());
        REQUIRE(neighbours.size() == actionsBackward.size());
        for (std::size_t i = 0; i < neighbours.size(); i++) {
            REQUIRE(gridaction::getActionFromNeighbours(referenceCell, neighbours[i]) == actionsForward[i]);
            REQUIRE(gridaction::getActionFromNeighbours(neighbours[i], referenceCell) == actionsBackward[i]);
        }
    }
}


/**
 * Test the getCellFromAction function 
 */
TEST_CASE("Get cell from Action", "[grid_action]") {
    enginetype::GridCell referenceCell = {2, 2};
    REQUIRE(gridaction::getCellFromAction(referenceCell, Action::noop) == referenceCell);
    REQUIRE(gridaction::getCellFromAction(referenceCell, Action::left) == (enginetype::GridCell){referenceCell.x - 1, referenceCell.y});
    REQUIRE(gridaction::getCellFromAction(referenceCell, Action::right) == (enginetype::GridCell){referenceCell.x + 1, referenceCell.y});
    REQUIRE(gridaction::getCellFromAction(referenceCell, Action::up) == (enginetype::GridCell){referenceCell.x, referenceCell.y - 1});
    REQUIRE(gridaction::getCellFromAction(referenceCell, Action::down) == (enginetype::GridCell){referenceCell.x, referenceCell.y + 1});
}