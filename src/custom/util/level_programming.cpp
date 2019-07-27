

#include "level_programming.h"


namespace levelprogramming {


/*
 * Spawns a diamond into an empty grid cell
 * Assumes diamond is Custom Element 1 i.e. element value 360
 */
void spawnYamYam() {
    // Get empty grid locations which a diamond can go
    std::vector<enginetype::GridCell> emptyGridCells;
    enginehelper::getEmptyGridCells(emptyGridCells);

    if (emptyGridCells.empty()) {
        PLOGI_(logwrap::FileLogger) << "No empty cells";
        return;
    }

    // Choose gridcell
    enginetype::GridCell emptyGridCell = emptyGridCells[RNG::getRandomNumber(emptyGridCells.size())];

    // Choose direction
    std::array<Action, 4> dirs = {Action::left, Action::right, Action::up, Action::down};
    Action dir = dirs[RNG::getRandomNumber(dirs.size())];


    // Spawn yamyam
    enginehelper::spawnElement(enginetype::FIELD_CUSTOM_2, dir, emptyGridCell);
    PLOGI_(logwrap::FileLogger) << "Adding yamyam at x=" << emptyGridCell.x << ", y=" << emptyGridCell.y;
}



/*
 * Spawns a diamond into an empty grid cell
 * Assumes diamond is Custom Element 1 i.e. element value 360
 */
void spawnDiamond() {
    // Get empty grid locations which a diamond can go
    std::vector<enginetype::GridCell> emptyGridCells;
    enginehelper::getEmptyGridCells(emptyGridCells);

    if (emptyGridCells.empty()) {
        PLOGI_(logwrap::FileLogger) << "No empty cells";
        return;
    }

    // Choose gridcell
    enginetype::GridCell emptyGridCell = emptyGridCells[RNG::getRandomNumber(emptyGridCells.size())];

    // Spawn diamond
    enginehelper::spawnElement(enginetype::FIELD_CUSTOM_1, enginetype::ENGINE_NOOP, emptyGridCell);
    PLOGI_(logwrap::FileLogger) << "Adding diamond at x=" << emptyGridCell.x << ", y=" << emptyGridCell.y;
}


void handleSurvivalLevel1() {
    // Check number of diamonds on screen
    int count_diamonds = enginehelper::countNumOfElement(enginetype::FIELD_CUSTOM_1);

    // Only add new diamond if all are collected
    // With every new diamond comes another Yamyam
    if (count_diamonds == 0) {
        spawnDiamond();
        spawnYamYam();
    }
}


/*
 * Spawn new elements based on level rules
 */
void spawnElements() {
    // Add proper level checks
    switch (enginehelper::getLevelNumber()) {
        case 1 :
            handleSurvivalLevel1();
            break;
        default :
            return;
    }
}

} // namespace levelprogramming