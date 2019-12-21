/**
 * @file: option_to_sprite.h
 *
 * @brief: An option which paths to the given sprite location.
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef OPTION_TO_ROCK_H
#define OPTION_TO_ROCK_H

// Standard Libary/STL
#include <deque>

// Includes
#include "base_option.h"
#include "engine_types.h"
#include "engine_types.h"

/**
 * Skeleton definition which each derived Option is based off of.
 *
 * All options implemented should be derived from this class. An option is a policy
 * which executes (possible non-deterministic) actions. The controller should determine
 * the type of option to run depending on the state. The method run() are required to 
 * be implemented.
 */
class OptionToRock : public BaseOption {

private:
    Action direction_;
    int item_;
    std::string optionStringName_ = "Path to boulder: ";

    void findRock();
    bool rockMoved();

public:
    
    OptionToRock(int spriteID, Action direction);

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

    std::string toString() const override;

    void setString(const std::string str);

};



#endif  //OPTION_TO_ROCK_H


