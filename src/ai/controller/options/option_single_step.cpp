/**
 * @file: option_single_step.cpp
 *
 * @brief: An option which performs a single step (action).
 * 
 * @author: Jake Tuero
 * Date: September 2019
 * Contact: tuero@ualberta.ca
 */

#include "option_single_step.h"

// Includes
#include "option_types.h"
#include "engine_helper.h"
#include "engine_types.h"
#include "logger.h"

using namespace enginehelper;


OptionSingleStep::OptionSingleStep(Action action, int numTimes) {
    optionType_ = OptionType::SingleAction;
    action_ = action;
    numTimes_ = numTimes;
    counter_ = 0;

    // Ensure non-positive number of times are not set
    if (numTimes <= 0) {
        PLOGE_(logger::FileLogger) << "Given parameter 'numTimes' should be positive. Defaulting to 1.";
        PLOGE_(logger::ConsoleLogger) << "Given parameter 'numTimes' should be positive. Defaulting to 1.";
        numTimes_ = 1;
    }
}


/**
 * Performs the saved action numTimes_ number of enginestate::getEngineUpdateRate() 
 * game ticks.
 */
bool OptionSingleStep::run() {
    for (int i = 0; i < enginestate::getEngineUpdateRate() * numTimes_; i++) {
        enginestate::setEnginePlayerAction(action_);
        enginestate::engineSimulateSingle();
    }
    return true;
}


/**
 * Return the stored action
 */
bool OptionSingleStep::getNextAction(Action &action) {
    // Reset counter (if necessary) and decrement
    counter_ = (counter_ == 0) ? numTimes_ - 1 : counter_ - 1;
    action = action_;
    return (counter_ == 0);
}


/**
 * Single action option is valid so long as the action it wants
 * to perform is moveable.
 */
bool OptionSingleStep::isValid() {
    return elementproperty::isActionMoveable(gridinfo::getPlayerPosition(), action_);
}


/**
 * String representation of the option and its characteristics.
 */
std::string OptionSingleStep::toString() const {
    return optionStringName + actioninfo::actionToString(action_);
}
