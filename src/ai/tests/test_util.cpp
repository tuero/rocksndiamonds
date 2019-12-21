/**
 * @file: test_util.cpp
 *
 * @brief: Util functions for common functionality while testing
 * 
 * @author: Jake Tuero
 * Date: December 2019
 * Contact: tuero@ualberta.ca
 */


// Includes
#include "engine_helper.h"

// Game headers
extern "C" {
    #include "../../libgame/system.h"       // options
    #include "../../init.h"                 // OpenAll()
    #include "../../game.h"                 // StartGameActions()
}


namespace testutil {

void loadTestLevelAndStart(int levelNum) {
    // Set levelset
    options.level_set = (char*)"test_levels";
    enginehelper::setLevelSet();

    // Reload the level dir tree
    OpenAll();

    // Load level and start game actions
    enginehelper::loadLevel(levelNum);
    StartGameActions(network.enabled, setup.autorecord, level.random_seed);
}

}