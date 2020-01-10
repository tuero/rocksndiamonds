/**
 * @file: option_to_exit.h
 *
 * @brief: An option which paths to the exit door.
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef OPTION_TO_EXIT_H
#define OPTION_TO_EXIT_H


// Includes
#include "base_option.h"
#include "engine_types.h"

/**
 * Skeleton definition which each derived Option is based off of.
 *
 * Option will path to the exit door. The pathing mechanism 
 * is to use shortest path (A*).
 */
class OptionToExit : public BaseOption {

private:
    int item_;
    int count_;
    enginetype::GridCell cell_;
    std::string optionStringName_ = "Path to exit: ";

public:
    
    OptionToExit(int spriteID);

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

    /**
     * Checks if the player can walk to the exit.
     * Player can only walk to the exit if the exit is open 
     * and the exit is pathable.
     */
    bool isValid() override;
    
    std::string toString() const override;
};



#endif  //OPTION_TO_EXIT_H


