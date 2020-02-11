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

// Standard Libary/STL
#include <vector>

// Includes
#include "engine_types.h"           // Gridcell, action
#include "engine_helper.h"          // Engine access functions
#include "rng.h"                    // RNG
#include "logger.h"                 // Logger

using namespace enginehelper;


namespace levelprogramming {

/**
 * Example custom level programming
 * Spawns an element into an empty grid cell
 */
void spawnElement(int element, int dir=enginetype::ENGINE_MV_NONE) {
    // Get empty grid locations which the element can appear
    std::vector<enginetype::GridCell> emptyGridCells = gridinfo::getEmptyGridCells();

    if (emptyGridCells.empty()) {
        PLOGV_(logger::FileLogger) << "No empty cells to spawn object: " << element;
        return;
    }

    // Choose gridcell using engine RNG for reproducibility
    enginetype::GridCell emptyGridCell = emptyGridCells[RNG::getRandomNumber(emptyGridCells.size())];

    // Spawn element
    gridinfo::spawnElement(element, dir, emptyGridCell);
    PLOGV_(logger::FileLogger) << "Adding element " << element << " at x=" << emptyGridCell.x << ", y="  << emptyGridCell.y;
}



/**
 * Handle custom level programming for each game tick.
 * For example, checking whether an item or enemy should be spawned.
 */
void customLevelProgrammingUpdate() {
    std::string levelSet = levelinfo::getLevelSet();

    // Your levelset name here
    if (levelSet == "") {
        switch (levelinfo::getLevelNumber()) {
            case 1 :
                // myCustomLevelProgrammingUpdateHere();
                break;
            default :
                return;
        }
    }
}


/**
 * Handle custom level programming at level start.
 */
void customLevelProgrammingStart() {
    std::string levelSet = levelinfo::getLevelSet();
    
    // Your levelset name here
    if (levelSet == "") {
        switch (levelinfo::getLevelNumber()) {
            case 1 :
                // myCustomLevelProgrammingStartHere();
                break;
            default :
                return;
        }
    }
}

} // namespace levelprogramming