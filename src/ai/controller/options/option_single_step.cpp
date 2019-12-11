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
#include "option_types.h"

// Engine
#include "../../engine/engine_helper.h"

// Logger
#include "../../util/logger.h"


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
 * Performs the saved action numTimes_ number of ENGINE_RESOLUTION 
 * game ticks.
 */
bool OptionSingleStep::run() {
    for (int i = 0; i < enginetype::ENGINE_RESOLUTION * numTimes_; i++) {
        enginehelper::setEnginePlayerAction(action_);
        enginehelper::engineSimulateSingle();
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
bool OptionSingleStep::isValid_() {
    return enginehelper::isActionMoveable(action_, enginehelper::getPlayerPosition());
}


/**
 * String representation of the option and its characteristics.
 */
std::string OptionSingleStep::toString() const {
    return optionStringName + enginehelper::actionToString(action_);
}
