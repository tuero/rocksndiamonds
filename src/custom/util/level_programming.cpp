

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
    // for (auto it = emptyGridCells.begin(); it != emptyGridCells.end(); ) {
    //     if (enginehelper::checkIfNeighbours(*it, player_cell)) {
    //         it = emptyGridCells.erase(it);
    //     }
    //     else {
    //         ++it;
    //     }
    // }

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
    enginehelper::setBoardDistancesL1(emptyGridCell);
    PLOGI_(logwrap::FileLogger) << "Adding diamond at x=" << emptyGridCell.x << ", y=" << emptyGridCell.y;
}


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
    // spawnDiamond();
    
    // Find diamond and set distances
    for (int y = 0; y < enginehelper::getLevelHeight(); y++) {
        for (int x = 0; x < enginehelper::getLevelWidth(); x++) {
            if (enginehelper::getGridItem({x, y}) == enginetype::FIELD_CUSTOM_1) {
                enginehelper::setBoardDistancesL1({x, y});
                return;
            }
        }
    }
}


/*
 * Spawn new elements based on level rules
 */
void customLevelProgrammingUpdate() {
    // Add proper level checks
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


/*
 * Spawn new elements based on level rules
 */
void customLevelProgrammingStart() {
    // Add proper level checks
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

} // namespace levelprogramming