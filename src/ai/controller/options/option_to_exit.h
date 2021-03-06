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


// Standard library and STL
#include <string>

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
    int item_;                          // Internal engine code defining exit
    std::string optionStringName_ = "Path to exit: ";

public:
    
    OptionToExit(int spriteID);

    /**
     * Run the action(s) defined by the option to get to the exit.
     * 
     * @return True if the option was able to perform without error, false otherwise.
     */
    bool run() override;

    /**
     * Get the next action to get to the exit.
     */
    bool getNextAction(Action &action) override;

    /**
     * Checks if the player can walk to the exit.
     * Player can only walk to the exit if the exit is open and the exit is pathable.
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



#endif  //OPTION_TO_EXIT_H


