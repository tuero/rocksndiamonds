/**
 * @file: engine_level_info.cpp
 *
 * @brief: Implementation of level information from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */


#include "engine_helper.h"

// Standard Libary/STL
#include <string>

// Includes
#include "logger.h"

// Game headers
extern "C" {
    #include "../../main.h"                 // level
    #include "../../files.h"                // LoadLevel
    #include "../../game.h"                 // game
    #include "../../init.h"                 // OpenAll
    #include "../../libgame/setup.h"        // LoadLevelInfo, SaveLevelSetup_LastSeries
    #include "../../libgame/system.h"       // options.levelset, leveldir_current
}


namespace enginehelper {
namespace levelinfo{

/**
 * Call engine functions to automatically load the levelset.
 */
void setLevelSet() {
    // Need to load levelset before openall
    // Maybe get setLevelset to check if replay, then get the levelset from there?
    // Kind of messy, need a better

    // No levelset given
    if (options.level_set == NULL) {return;}

    std::string level_set(options.level_set);

    try{
        // Initialize leveldir_current and related objects
        LoadLevelInfo();
        PLOGI_(logger::FileLogger) << "Setting levelset: \"" << level_set << "\"";
        PLOGI_(logger::ConsoleLogger) << "Setting levelset: \"" << level_set << "\"";

        // Set levelset to save
        leveldir_current->fullpath = options.level_set;
        leveldir_current->subdir = options.level_set;
        leveldir_current->identifier = options.level_set;
        leveldir_current->in_user_dir = false;

        // Save the levelset
        // We save because on startup, the previously saved levelset is loaded
        SaveLevelSetup_LastSeries();
        LoadLevelSetup_SeriesInfo();

        SaveLevelSetup_LastSeries();
        SaveLevelSetup_SeriesInfo();
    }
    catch (...){
        PLOGE_(logger::FileLogger) << "Something went wrong trying to load levelset " << level_set;
        PLOGE_(logger::ConsoleLogger) << "Something went wrong trying to load levelset " << level_set;
    }
}

/**
 * Get the levelset currently set in the engine.
 */
const std::string getLevelSet() {
    return leveldir_current->subdir;
}


/**
 * Call engine function to load the given level.
 */
void loadLevel(int level_num) {
    options.level_number = level_num;
    PLOGI_(logger::FileLogger) << "Loading level: " << level_num;
    PLOGI_(logger::ConsoleLogger) << "Loading level: " << level_num;
    LoadLevel(level_num);
}


/**
 * Get the level number loaded in the level struct
 */
int getLevelNumber() {
    return level.file_info.nr;
}


/**
 * Restart the level.
 * Game is reinitialized using the current loaded level.
 */
void restartLevel() {    
    InitGame();
    gridinfo::initSpriteIDs();
}


/**
 * Get the level height of the level currently loaded in the engine.
 */
int getLevelHeight() {
    return level.fieldy;
}


/**
 * Get the level width of the level currently loaded in the engine.
 */
int getLevelWidth() {
    return level.fieldx;
}


/**
 * Get the number of gems needed to open the exit.
 */
int getLevelGemsNeeded() {
    return level.gems_needed;
}


/**
 * Get the number of remaining gems needed to open the exit.
 */
int getLevelRemainingGemsNeeded() {
    return game.gems_still_needed;
}

} // namespace levelinfo
} //namespace enginehelper