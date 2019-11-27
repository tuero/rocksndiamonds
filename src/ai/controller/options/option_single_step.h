/**
 * @file: option_single_step.h
 *
 * @brief: An option which performs a single step (action).
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef OPTION_SINGLE_STEP_H
#define OPTION_SINGLE_STEP_H


#include "base_option.h"
#include "../../engine/action.h"

/**
 * Represents option of performing one specific action a number of times.
 * 
 * Derived class of BaseOption. Only one action (UP, DOWN, LEFT, RIGHT, NOOP)
 * can be represented by the option, and the option can perform that action
 * any number of times.
 */
class OptionSingleStep : public BaseOption {

private:
    Action action_;
    int numTimes_;
    std::string optionStringName = "Single action: ";

public:
    
    OptionSingleStep(Action action, int numTimes);

    /**
     * Performs the saved action numTimes_ number of ENGINE_RESOLUTION 
     * game ticks.
     * 
     * @return True if there was no failure during performing the action.
     */
    bool run() override;
    
    /**
     * Return the stored action.
     * 
     * Internal counter is decremented on each call. Once the counter reaches 0 
     * (signifying the option is complete), the counter gets reset and true
     * is returned.
     * 
     * @param action Reference to action to set.
     * @return True if the action has been sent numTimes_ times, false otherwise.
     */
    bool getNextAction(Action &action) override;

    /**
     * Single action option is valid so long as the action it wants
     * to perform is moveable.
     * 
     * @return True if the action the option represents is moveable.
     */
    bool isValid_() override;

    /**
     * String representation of the option and its characteristics.
     * 
     * @return String representation of option.
     */
    std::string toString() const override;
};



#endif  //OPTION_SINGLE_STEP_H


