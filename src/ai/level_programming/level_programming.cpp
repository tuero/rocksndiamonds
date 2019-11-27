/**
 * @file: level_programming.cpp
 *
 * @brief: Handle custom level programming such as spawning new elements.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "level_programming.h"

#include <vector>
#include <array>

// Engine
#include "../engine/engine_helper.h"
#include "../engine/engine_types.h"
#include "../engine/action.h"

// Logging
#include "../util/logger.h"


namespace levelprogramming {


/*
 * Spawns a diamond into an empty grid cell
 * Assumes diamond is Custom Element 1 i.e. element value 360
 */
void spawnYamYam() {
    // Get empty grid locations which a diamond can go
    std::vector<enginetype::GridCell> emptyGridCells = enginehelper::getEmptyGridCells();

    if (emptyGridCells.empty()) {
        PLOGI_(logger::FileLogger) << "No empty cells to spawn YamYam.";
        return;
    }

    // Remove gridcells adjacent to player (so player has time to react)
    // !<--> TODO: make this a function
    enginetype::GridCell player_cell = enginehelper::getPlayerPosition();
    for (auto it = emptyGridCells.begin(); it != emptyGridCells.end(); ) {
        if (enginehelper::getL1Distance(*it, player_cell) <= 3) {
            it = emptyGridCells.erase(it);
        }
        else {
            ++it;
        }
    }

    // Choose gridcell
    enginetype::GridCell emptyGridCell = emptyGridCells[RNG::getRandomNumber(emptyGridCells.size())];

    // Choose direction
    std::array<Action, 4> dirs = {Action::left, Action::right, Action::up, Action::down};
    Action dir = dirs[RNG::getRandomNumber(dirs.size())];


    // Spawn yamyam
    enginehelper::spawnElement(enginetype::FIELD_CUSTOM_2, dir, emptyGridCell);
    PLOGI_(logger::FileLogger) << "Adding yamyam at x=" << emptyGridCell.x << ", y="  << emptyGridCell.y;
}



/*
 * Spawns a diamond into an empty grid cell
 * Assumes diamond is Custom Element 1 i.e. element value 360
 */
void spawnDiamond() {
    // Get empty grid locations which a diamond can go
    std::vector<enginetype::GridCell> emptyGridCells = enginehelper::getEmptyGridCells();

    if (emptyGridCells.empty()) {
        PLOGI_(logger::FileLogger) << "No empty cells to spawn Diamond.";
        return;
    }

    // Choose gridcell
    enginetype::GridCell emptyGridCell = emptyGridCells[RNG::getRandomNumber(emptyGridCells.size())];

    // Spawn diamond
    enginehelper::spawnElement(enginetype::FIELD_CUSTOM_1, enginetype::ENGINE_NOOP, emptyGridCell);
    PLOGI_(logger::FileLogger) << "Adding diamond at x=" << emptyGridCell.x << ", y="  << emptyGridCell.y;
}


/*
 * The survival levelset 
 * For each diamond the agent collects, another yamyam will spawn.
 */
void handleSurvivalUpdate() {
    // Check number of diamonds on screen
    int count_diamonds = enginehelper::countNumOfElement(enginetype::FIELD_CUSTOM_1);
    int count_yamyam = enginehelper::countNumOfElement(enginetype::FIELD_CUSTOM_2);

    // Only add new diamond if all are collected
    // With every new diamond comes another Yamyam
    if (count_diamonds == 0) {
        spawnDiamond();
        if (count_yamyam < 15) {spawnYamYam();}
    }
}

void handleSurvivalLevelStart() {   
    
}


/*
 * Handle custom level programming for each game tick.
 * 
 * For example, checking whether an item or enemy should be spawned.
 */
void customLevelProgrammingUpdate() {
    // Add proper level checks
    std::string levelSet = enginehelper::getLevelSet();

    if (levelSet == "custom_survival") {
        switch (enginehelper::getLevelNumber()) {
            case 1 :
            case 2 :
            case 5 :
            case 6 :
                handleSurvivalUpdate();
                break;
            default :
                return;
        }
    }
}


/*
 * Handle custom level programming at level start.
 */
void customLevelProgrammingStart() {
    // Add proper level checks
    std::string levelSet = enginehelper::getLevelSet();
    
    if (levelSet == "custom_survival") {
        switch (enginehelper::getLevelNumber()) {
            case 1 :
            case 2 :
            case 5 :
            case 6 :
                handleSurvivalLevelStart();
                break;
            default :
                return;
        }
    }
}

} // namespace levelprogramming