/**
 * @file: engine_level_info.h
 *
 * @brief: Interface for controllers to level information from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

// ------------- Includes  -------------

// Standard Libary/STL
#include <string>

// Include
#include "engine_types.h"


namespace enginehelper {
namespace levelinfo {

    /**
     * Call engine functions to automatically load the levelset.
     *
     * This is used when supplying the command line argument -levelset <LEVELSET>, so that
     * a specific levelset/level can be preloaded and started without having to manually 
     * select the levelset/level using the GUI.
     */
    void setLevelSet();

    /**
     * Get the levelset currently set in the engine.
     *
     * @return The string name of the levelset.
     */
    const std::string getLevelSet();

    /**
     * Call engine function to load the given level.
     *
     * The struct options.level_number is referenced by the engine when loading the
     * level by LoadLevel(int).
     *
     * The engine function LoadLevel(int) will attempt to load the level, regardless if it is 
     * valid. If the level doesn't exist, LoadLevel(int) falls back to using a default
     * empty level.
     *
     * @param level_num The integer level number.
     */
    void loadLevel(int level_num);

    /**
     * Get the level number loaded in the level struct
     *
     * @return The level number
     */
    int getLevelNumber();

    /**
     * Restart the level.
     * Game is reinitialized using the current loaded level.
     */
    void restartLevel();

    /**
     * Get the level height of the level currently loaded in the engine.
     *
     * @return The level height as measured in grid tiles.
     */
    int getLevelHeight();

    /**
     * Get the level width of the level currently loaded in the engine.
     *
     * @return The level width as measured in grid tiles.
     */
    int getLevelWidth();

    /**
     * Get the number of gems needed to open the exit.
     * 
     * @return The number of gems.
     */
    int getLevelGemsNeeded();

    /**
     * Get the number of remaining gems needed to open the exit.
     * 
     * @return the count of gems remaining to open the exit.
     */
    int getLevelRemainingGemsNeeded();

} //namespace levelinfo
} //namespace enginehelper