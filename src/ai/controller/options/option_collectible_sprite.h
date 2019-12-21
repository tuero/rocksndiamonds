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
#include <deque>

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
    int item_;
    int count_;
    std::string optionStringName_ = "Path to sprite: ";

public:
    
    OptionCollectibleSprite(int spriteID);

    /**
     * Set action which agent should immediately take.
     * 
     * Called when the currentSolution is empty. Agent always executes the next action
     * from currentSolution. If conducting search on the future state while the agent 
     * is conducting the current action, the controller should set currentSolution to
     * the forwardSolution.
     */
    bool run() override;

    bool getNextAction(Action &action) override;

    bool isValid_() override;

    bool isComplete() override;

    std::string toString() const override;
};



#endif  //OPTION_COLLECTIBLE_SPRITE_H


