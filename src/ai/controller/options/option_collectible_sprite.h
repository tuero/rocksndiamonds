/**
 * @file: option_collectible_sprite.h
 *
 * @brief: An option which paths to the given sprite location.
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef OPTION_COLLECTIBLE_SPRITE_H
#define OPTION_COLLECTIBLE_SPRITE_H

// Standard Libary/STL
#include <string>

// Includes
#include "base_option.h"
#include "engine_types.h"

/**
 * Skeleton definition which each derived Option is based off of.
 *
 * Option will path to the represented collectible sprite. Examples of 
 * collectible sprites are diamonds and emeralds. The pathing mechanism 
 * is to use shortest path (A*).
 */
class OptionCollectibleSprite : public BaseOption {

private:
    int item_;                              // Internal engine code defining collectible sprite.
    std::string optionStringName_ = "Path to sprite: ";

public:
    
    OptionCollectibleSprite(int spriteID);

    /**
     * Run the action(s) defined by the option to get to the collectible sprite.
     * 
     * @return True if the option was able to perform without error, false otherwise.
     */
    bool run() override;

    /**
     * Get the next action to get to the collectible sprite.
     */
    bool getNextAction(Action &action) override;

    /**
     * Checks if the player can walk to the collectible sprite.
     * Player can only walk to the sprite if it is pathable.
     * 
     * @return True if the option is valid, false otherwise.
     */
    bool isValid() override;

    /**
     * Convert to human readable format (spriteID, location, etc.)
     * 
     * @return String representation of option.
     */
    std::string toString() const override;
};



#endif  //OPTION_COLLECTIBLE_SPRITE_H


